/* eslint-disable indent */
(function() {
    function getMediaStreamAudioTracks(mediaSource) {
        return mediaSource.MediaStreams.filter(function (s) {
            return s.Type === 'Audio';
        });
    }

    class mpvVideoPlayer {
        constructor({ events, loading, appRouter, globalize, appHost, appSettings, confirm, dashboard }) {
            this.events = events;
            this.loading = loading;
            this.appRouter = appRouter;
            this.globalize = globalize;
            this.appHost = appHost;
            this.appSettings = appSettings;

            this.setTransparency = dashboard.default.setBackdropTransparency.bind(dashboard);

            /**
             * @type {string}
             */
            this.name = 'MPV Video Player';
            /**
             * @type {string}
             */
            this.type = 'mediaplayer';
            /**
             * @type {string}
             */
            this.id = 'mpvvideoplayer';
            this.syncPlayWrapAs = 'htmlvideoplayer';
            this.priority = -1;
            this.useFullSubtitleUrls = true;
            /**
             * @type {boolean}
             */
            this.isFetching = false;

            /**
             * @type {HTMLDivElement | null | undefined}
             */
            this._videoDialog = undefined;
            /**
             * @type {number | undefined}
             */
            this._subtitleTrackIndexToSetOnPlaying = undefined;
            /**
             * @type {number | null}
             */
            this._audioTrackIndexToSetOnPlaying = undefined;
            /**
             * @type {boolean | undefined}
             */
            this._showTrackOffset = undefined;
            /**
             * @type {number | undefined}
             */
            this._currentTrackOffset = undefined;
            /**
             * @type {string[] | undefined}
             */
            this._supportedFeatures = undefined;
            /**
             * @type {string | undefined}
             */
            this._currentSrc = undefined;
            /**
             * @type {boolean | undefined}
             */
            this._started = undefined;
            /**
             * @type {boolean | undefined}
             */
            this._timeUpdated = undefined;
            /**
             * @type {number | null | undefined}
             */
            this._currentTime = undefined;
            /**
             * @private (used in other files)
             * @type {any | undefined}
             */
            this._currentPlayOptions = undefined;
            /**
             * @type {any | undefined}
             */
            this._lastProfile = undefined;
            /**
             * @type {number | undefined}
             */
            this._duration = undefined;
            /**
             * @type {boolean}
             */
            this._paused = false;
            /**
             * @type {int}
             */
            this._volume = 100;
            /**
             * @type {boolean}
             */
            this._muted = false;
            /**
             * @type {float}
             */
            this._playRate;
            /**
             * @type {boolean}
             */
            this._hasConnection = false;

            /**
             * @private
             */
            this.onEnded = () => {
                this.onEndedInternal();
            };

            /**
             * @private
             */
            this.onTimeUpdate = (time) => {
                if (time && !this._timeUpdated) {
                    this._timeUpdated = true;
                }

                this._currentTime = time;
                this.events.trigger(this, 'timeupdate');
            };

            /**
             * @private
             */
            this.onNavigatedToOsd = () => {
                const dlg = this._videoDialog;
                if (dlg) {
                    dlg.style.zIndex = 'unset';
                }
            };

            /**
             * @private
             */
            this.onPlaying = () => {
                if (!this._started) {
                    this._started = true;

                    this.loading.hide();

                    const volume = this.getSavedVolume() * 100;
                    this.setVolume(volume, false);

                    this.setPlaybackRate(this.getPlaybackRate());

                    if (this._currentPlayOptions.fullscreen) {
                        this.appRouter.showVideoOsd().then(this.onNavigatedToOsd);
                    } else {
                        this.setTransparency('backdrop');
                        this._videoDialog.dlg.style.zIndex = 'unset';
                    }

                    // Need to override default style.
                    this._videoDialog.style.setProperty('background', 'transparent', 'important');
                }

                if (this._paused) {
                    this._paused = false;
                    this.events.trigger(this, 'unpause');
                }

                this.events.trigger(this, 'playing');
            };

            /**
             * @private
             */
            this.onPause = () => {
                this._paused = true;
                // For Syncplay ready notification
                this.events.trigger(this, 'pause');
            };

            this.onWaiting = () => {
                this.events.trigger(this, 'waiting');
            };

            /**
             * @private
             * @param e {Event} The event received from the `<video>` element
             */
            this.onError = async (error) => {
                this.removeMediaDialog();
                console.error(`media error: ${error}`);

                const errorData = {
                    type: 'mediadecodeerror'
                };

                try {
                    await confirm({
                        title: "Playback Failed",
                        text: `Playback failed with error "${error}". Retry with transcode? (Note this may hang the player.)`,
                        cancelText: "Cancel",
                        confirmText: "Retry"
                    });
                } catch (ex) {
                    // User declined retry
                    errorData.streamInfo = {
                        // Prevent jellyfin-web retrying with transcode
                        // which crashes the player
                        mediaSource: {
                            SupportsTranscoding: false
                        }
                    };
                }

                this.events.trigger(this, 'error', [errorData]);
            };

            this.onDuration = (duration) => {
                this._duration = duration;
            };
        }

        currentSrc() {
            return this._currentSrc;
        }

        async play(options) {
            this._started = false;
            this._timeUpdated = false;
            this._currentTime = null;

            this.resetSubtitleOffset();
            this.loading.show();
            window.api.power.setScreensaverEnabled(false);
            const elem = await this.createMediaElement(options);
            return await this.setCurrentSrc(elem, options);
        }

        getSavedVolume() {
            return this.appSettings.get('volume') || 1;
        }

        /**
         * @private
         */
        getRelativeIndexByType(mediaStreams, jellyIndex, streamType) {
            let relIndex = 1;
            for (const source of mediaStreams) {
                if (source.Type != streamType || source.IsExternal) {
                    continue;
                }

                if (source.Index == jellyIndex) {
                    return relIndex;
                }

                relIndex += 1;
            }

            return null;
        }

        /**
         * @private
         */
        getStreamByIndex(mediaStreams, jellyIndex) {
            for (const source of mediaStreams) {
                if (source.Index == jellyIndex) {
                    return source;
                }
            }

            return null;
        }

        /**
         * @private
         */
        getSubtitleParam() {
            const options = this._currentPlayOptions;

            if (this._subtitleTrackIndexToSetOnPlaying != null && this._subtitleTrackIndexToSetOnPlaying >= 0) {
                const initialSubtitleStream = this.getStreamByIndex(options.mediaSource.MediaStreams, this._subtitleTrackIndexToSetOnPlaying);
                if (!initialSubtitleStream || initialSubtitleStream.DeliveryMethod === 'Encode') {
                    this._subtitleTrackIndexToSetOnPlaying = -1;
                } else if (initialSubtitleStream.DeliveryMethod === 'External') {
                    return '#,' + initialSubtitleStream.DeliveryUrl;
                }
            }

            if (this._subtitleTrackIndexToSetOnPlaying == -1 || this._subtitleTrackIndexToSetOnPlaying == null) {
                return '';
            }

            const subtitleRelIndex = this.getRelativeIndexByType(
                options.mediaSource.MediaStreams,
                this._subtitleTrackIndexToSetOnPlaying,
                'Subtitle'
            );

            return subtitleRelIndex != null
                ? '#' + subtitleRelIndex
                : '';
        }

        /**
         * @private
         */
        getAudioParam() {
            const options = this._currentPlayOptions;

            if (this._audioTrackIndexToSetOnPlaying != null && this._audioTrackIndexToSetOnPlaying >= 0) {
                const initialAudioStream = this.getStreamByIndex(options.mediaSource.MediaStreams, this._audioTrackIndexToSetOnPlaying);
                if (!initialAudioStream) {
                    return '#1';
                }
            }

            if (this._audioTrackIndexToSetOnPlaying == -1 || this._audioTrackIndexToSetOnPlaying == null) {
                return '#1';
            }

            const audioRelIndex = this.getRelativeIndexByType(
                options.mediaSource.MediaStreams,
                this._audioTrackIndexToSetOnPlaying,
                'Audio'
            );

            return audioRelIndex != null
                ? '#' + audioRelIndex
                : '#1';
        }

        tryGetFramerate(options) {
            if (options.mediaSource && options.mediaSource.MediaStreams) {
                for (let stream of options.mediaSource.MediaStreams) {
                    if (stream.Type == "Video") {
                        return stream.RealFrameRate || stream.AverageFrameRate || null;
                    }
                }
            }
        }

        /**
         * @private
         */
        setCurrentSrc(elem, options) {
            return new Promise((resolve) => {
                const val = options.url;
                this._currentSrc = val;
                console.debug(`playing url: ${val}`);

                // Convert to seconds
                const ms = (options.playerStartPositionTicks || 0) / 10000;
                this._currentPlayOptions = options;
                this._subtitleTrackIndexToSetOnPlaying = options.mediaSource.DefaultSubtitleStreamIndex == null ? -1 : options.mediaSource.DefaultSubtitleStreamIndex;
                this._audioTrackIndexToSetOnPlaying = options.playMethod === 'Transcode' ? null : options.mediaSource.DefaultAudioStreamIndex;

                const streamdata = {type: 'video', headers: {'User-Agent': 'JellyfinMediaPlayer'}, metadata: options.item, media: {}};
                const fps = this.tryGetFramerate(options);
                if (fps) {
                    streamdata.frameRate = fps;
                }

                const player = window.api.player;
                player.load(val,
                    { startMilliseconds: ms, autoplay: true },
                    streamdata,
                    this.getAudioParam(),
                    this.getSubtitleParam(),
                    resolve);
            });
        }

        setSubtitleStreamIndex(index) {
            this._subtitleTrackIndexToSetOnPlaying = index;
            window.api.player.setSubtitleStream(this.getSubtitleParam());
        }

        resetSubtitleOffset() {
            this._currentTrackOffset = 0;
            this._showTrackOffset = false;
            window.api.player.setSubtitleDelay(0);
        }

        enableShowingSubtitleOffset() {
            this._showTrackOffset = true;
        }

        disableShowingSubtitleOffset() {
            this._showTrackOffset = false;
        }

        isShowingSubtitleOffsetEnabled() {
            return this._showTrackOffset;
        }

        setSubtitleOffset(offset) {
            const offsetValue = parseFloat(offset);
            this._currentTrackOffset = offsetValue;
            window.api.player.setSubtitleDelay(Math.round(offsetValue * 1000));
        }

        getSubtitleOffset() {
            return this._currentTrackOffset;
        }

        /**
         * @private
         */
        isAudioStreamSupported() {
            return true;
        }

        /**
         * @private
         */
        getSupportedAudioStreams() {
            const profile = this._lastProfile;

            return getMediaStreamAudioTracks(this._currentPlayOptions.mediaSource).filter((stream) => {
                return this.isAudioStreamSupported(stream, profile);
            });
        }

        setAudioStreamIndex(index) {
            this._audioTrackIndexToSetOnPlaying = index;
            const streams = this.getSupportedAudioStreams();

            if (streams.length < 2) {
                // If there's only one supported stream then trust that the player will handle it on it's own
                return;
            }

            window.api.player.setAudioStream(this.getAudioParam());
        }

        onEndedInternal() {
            const stopInfo = {
                src: this._currentSrc
            };

            this.events.trigger(this, 'stopped', [stopInfo]);

            this._currentTime = null;
            this._currentSrc = null;
            this._currentPlayOptions = null;
        }

        stop(destroyPlayer) {
            window.api.player.stop();
            window.api.power.setScreensaverEnabled(true);

            this.onEndedInternal();

            if (destroyPlayer) {
                this.destroy();
            }
            return Promise.resolve();
        }

        removeMediaDialog() {
            this.loading.hide();
            window.api.player.stop();
            window.api.power.setScreensaverEnabled(true);

            this.setTransparency('none');

            document.body.classList.remove('hide-scroll');

            const dlg = this._videoDialog;
            if (dlg) {
                this._videoDialog = null;
                dlg.parentNode.removeChild(dlg);
            }

            // Only supporting QtWebEngine here
            if (document.webkitIsFullScreen && document.webkitExitFullscreen) {
                document.webkitExitFullscreen();
            }
        }

        destroy() {
            this.removeMediaDialog();

            const player = window.api.player;
            this._hasConnection = false;
            player.playing.disconnect(this.onPlaying);
            player.positionUpdate.disconnect(this.onTimeUpdate);
            player.finished.disconnect(this.onEnded);
            this._duration = undefined;
            player.updateDuration.disconnect(this.onDuration);
            player.error.disconnect(this.onError);
            player.paused.disconnect(this.onPause);
        }

        /**
         * @private
         */
        createMediaElement(options) {
            const dlg = document.querySelector('.videoPlayerContainer');

            if (!dlg) {
                this.loading.show();

                const dlg = document.createElement('div');

                dlg.classList.add('videoPlayerContainer');
                dlg.style.position = 'fixed';
                dlg.style.top = 0;
                dlg.style.bottom = 0;
                dlg.style.left = 0;
                dlg.style.right = 0;
                dlg.style.display = 'flex';
                dlg.style.alignItems = 'center';

                if (options.fullscreen) {
                    dlg.style.zIndex = 1000;
                }

                const html = '';

                dlg.innerHTML = html;

                document.body.insertBefore(dlg, document.body.firstChild);
                this._videoDialog = dlg;
                const player = window.api.player;
                if (!this._hasConnection) {
                    this._hasConnection = true;
                    player.playing.connect(this.onPlaying);
                    player.positionUpdate.connect(this.onTimeUpdate);
                    player.finished.connect(this.onEnded);
                    player.updateDuration.connect(this.onDuration);
                    player.error.connect(this.onError);
                    player.paused.connect(this.onPause);
                }

                if (options.fullscreen) {
                    // At this point, we must hide the scrollbar placeholder, so it's not being displayed while the item is being loaded
                    document.body.classList.add('hide-scroll');
                }
                return Promise.resolve();
            } else {
                // we need to hide scrollbar when starting playback from page with animated background
                if (options.fullscreen) {
                    document.body.classList.add('hide-scroll');
                }

                return Promise.resolve();
            }
        }

    /**
     * @private
     */
    canPlayMediaType(mediaType) {
        return (mediaType || '').toLowerCase() === 'video';
    }

    /**
     * @private
     */
    supportsPlayMethod() {
        return true;
    }

    /**
     * @private
     */
    getDeviceProfile(item, options) {
        if (this.appHost.getDeviceProfile) {
            return this.appHost.getDeviceProfile(item, options);
        }

        return Promise.resolve({});
    }

    /**
     * @private
     */
    static getSupportedFeatures() {
        return ['PlaybackRate', 'SetAspectRatio'];
    }

    supports(feature) {
        if (!this._supportedFeatures) {
            this._supportedFeatures = mpvVideoPlayer.getSupportedFeatures();
        }

        return this._supportedFeatures.includes(feature);
    }

    // Save this for when playback stops, because querying the time at that point might return 0
    currentTime(val) {
        if (val != null) {
            window.api.player.seekTo(val);
            return;
        }

        return this._currentTime;
    }

    currentTimeAsync() {
        return new Promise((resolve) => {
            window.api.player.getPosition(resolve);
        });
    }

    duration() {
        if (this._duration) {
            return this._duration;
        }

        return null;
    }

    canSetAudioStreamIndex() {
        return true;
    }

    static onPictureInPictureError(err) {
        console.error(`Picture in picture error: ${err}`);
    }

    setPictureInPictureEnabled() {}

    isPictureInPictureEnabled() {
        return false;
    }

    isAirPlayEnabled() {
        return false;
    }

    setAirPlayEnabled() {}

    setBrightness() {}

    getBrightness() {
        return 100;
    }

    seekable() {
        return Boolean(this._duration);
    }

    pause() {
        window.api.player.pause();
        window.api.power.setScreensaverEnabled(true);
    }

    // This is a retry after error
    resume() {
        this._paused = false;
        window.api.player.play();
    }

    unpause() {
        window.api.player.play();
        window.api.power.setScreensaverEnabled(false);
    }

    paused() {
        return this._paused;
    }

    setPlaybackRate(value) {
        let playSpeed = +value; //this comes as a string from player force int for now
        this._playRate = playSpeed;
        window.api.player.setPlaybackRate(playSpeed * 1000);
    }

    getPlaybackRate() {
        if(!this._playRate) //On startup grab default
        {
            let playRate = window.jmpInfo.settings.video.default_playback_speed;

            if(!playRate) //fallback if default missing
                playRate = 1;

            this._playRate = playRate;
        }
        return this._playRate;
    }

    getSupportedPlaybackRates() {
        return [{
            name: '0.5x',
            id: 0.5
        }, {
            name: '0.75x',
            id: 0.75
        }, {
            name: '1x',
            id: 1.0
        }, {
            name: '1.25x',
            id: 1.25
        }, {
            name: '1.5x',
            id: 1.5
        }, {
            name: '1.75x',
            id: 1.75
        }, {
            name: '2x',
            id: 2.0
        }];
    }

    saveVolume(value) {
        if (value) {
            this.appSettings.set('volume', value);
        }
    }

    setVolume(val, save = true) {
        val = Number(val);
        if (!isNaN(val)) {
            this._volume = val;
            if (save) {
                this.saveVolume(val / 100);
                this.events.trigger(this, 'volumechange');
            }
            window.api.player.setVolume(val);
        }
    }

    getVolume() {
        return this._volume;
    }

    volumeUp() {
        this.setVolume(Math.min(this.getVolume() + 2, 100));
    }

    volumeDown() {
        this.setVolume(Math.max(this.getVolume() - 2, 0));
    }

    setMute(mute, triggerEvent = true) {
        this._muted = mute;
        window.api.player.setMuted(mute);
        if (triggerEvent) {
            this.events.trigger(this, 'volumechange');
        }
    }

    isMuted() {
        return this._muted;
    }

    togglePictureInPicture() {
    }

    toggleAirPlay() {
    }

    getBufferedRanges() {
        return [];
    }

    getStats() {
        const playOptions = this._currentPlayOptions || [];
        const categories = [];

        if (!this._currentPlayOptions) {
            return Promise.resolve({
                categories: categories
            });
        }

        const mediaCategory = {
            stats: [],
            type: 'media'
        };
        categories.push(mediaCategory);

        if (playOptions.url) {
            //  create an anchor element (note: no need to append this element to the document)
            let link = document.createElement('a');
            //  set href to any path
            link.setAttribute('href', playOptions.url);
            const protocol = (link.protocol || '').replace(':', '');

            if (protocol) {
                mediaCategory.stats.push({
                    label: this.globalize.translate('LabelProtocol'),
                    value: protocol
                });
            }

            link = null;
        }

        mediaCategory.stats.push({
            label: this.globalize.translate('LabelStreamType'),
            value: 'Video'
        });

        const videoCategory = {
            stats: [],
            type: 'video'
        };
        categories.push(videoCategory);

        const audioCategory = {
            stats: [],
            type: 'audio'
        };
        categories.push(audioCategory);

        return Promise.resolve({
            categories: categories
        });
    }

    getSupportedAspectRatios() {
        const options = window.jmpInfo.settingsDescriptions.video.find(x => x.key == 'aspect').options;
        const current = window.jmpInfo.settings.video.aspect;

        const getOptionName = (option) => {
            const canTranslate = {
                'normal': 'Auto',
                'zoom': 'AspectRatioCover',
                'stretch': 'AspectRatioFill',
            }
            const name = option.replace('video.aspect.', '');
            return canTranslate[name]
                ? this.globalize.translate(canTranslate[name])
                : name;
        }

        return options.map(x => ({
            id: x.value,
            name: getOptionName(x.title),
            selected: x.value == current
        }));
    }

    getAspectRatio() {
        return window.jmpInfo.settings.video.aspect;
    }

    setAspectRatio(value) {
        window.jmpInfo.settings.video.aspect = value;
    }
    }
/* eslint-enable indent */

window._mpvVideoPlayer = mpvVideoPlayer;
})();
