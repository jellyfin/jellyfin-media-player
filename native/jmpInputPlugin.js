const remap = {
    "play_pause": "playpause",
    "seek_forward": "fastforward",
    "seek_backward": "rewind",
    "host:fullscreen": "togglefullscreen",
    "cycle_audio": "changeaudiotrack",
    "cycle_subtitles": "changesubtitletrack",
    "increase_volume": "volumeup",
    "decrease_volume": "volumedown",
    "step_backward": "previouschapter",
    "step_forward": "nextchapter",
    "enter": "select",
}

class jmpInputPlugin {
    constructor({ inputManager, playbackManager }) {
        this.name = 'JMP Input Plugin';
        this.type = 'input';
        this.id = 'jmpInputPlugin';

        this.durationCheckInterval = null;
        this.positionUpdateInterval = null;
        this.attachedPlayer = null;

        (async () => {
            const api = await window.apiPromise;

            api.input.hostInput.connect((actions) => {
                actions.forEach(action => {
                    if (action === 'shuffle') {
                        playbackManager.setQueueShuffleMode('Shuffle');
                    } else if (action === 'sorted') {
                        playbackManager.setQueueShuffleMode('Sorted');
                    } else if (action === 'previous') {
                        const currentPlayer = playbackManager._currentPlayer;
                        if (currentPlayer && playbackManager.isPlayingAudio(currentPlayer)) {
                            const currentTime = playbackManager.currentTime(currentPlayer);
                            const currentIndex = playbackManager.getCurrentPlaylistIndex(currentPlayer);

                            if (currentTime >= 5 * 1000 || currentIndex <= 0) {
                                playbackManager.seekPercent(0, currentPlayer);
                            } else {
                                playbackManager.previousTrack(currentPlayer);
                            }
                        } else if (currentPlayer) {
                            playbackManager.previousTrack(currentPlayer);
                        }
                    } else {
                        if (remap.hasOwnProperty(action)) {
                            action = remap[action];
                        }
                        inputManager.handleCommand(action, {});
                    }
                });
            });

            // Monitor queue changes and update MPRIS navigation capabilities
            const updateQueueState = function() {
                try {
                    if (!api || !api.mpris) {
                        return;
                    }

                    // Access the internal playlist directly
                    const playlist = playbackManager._playQueueManager?.getPlaylist();
                    if (!playlist || !Array.isArray(playlist)) {
                        return;
                    }

                    const currentIndex = playbackManager._playQueueManager?.getCurrentPlaylistIndex();
                    if (currentIndex === undefined || currentIndex === null || currentIndex < 0) {
                        return;
                    }

                    const canNext = currentIndex < playlist.length - 1;

                    // For music: Previous always enabled (goes to prev track or restarts current)
                    // For video: Previous only if there's a previous item
                    const state = playbackManager.getPlayerState();
                    const isMusic = state?.NowPlayingItem?.MediaType === 'Audio';
                    const canPrevious = isMusic ? true : currentIndex > 0;

                    api.mpris.notifyQueueChange(canNext, canPrevious);
                } catch (e) {
                    console.error('MPRIS: Error in updateQueueState:', e);
                }
            };

            // Listen for shuffle and repeat mode changes and notify native player
            // Attach listeners on playbackstart event
            let globalListenersAttached = false;
            let lastReportedPosition = 0;  // Track position to detect seeks

            window.Events.on(playbackManager, 'playbackstart', (e, player) => {
                console.log('MPRIS: playbackstart event fired, player:', player);
                if (!player) return;

                // Notify MPRIS of metadata when playback starts
                const state = playbackManager.getPlayerState();
                console.log('MPRIS: state:', state, 'NowPlayingItem:', state?.NowPlayingItem);
                if (state && state.NowPlayingItem) {
                    console.log('MPRIS: Sending metadata on playbackstart');

                    // Get server address from global ApiClient
                    let serverAddress = '';
                    if (window.ApiClient && typeof window.ApiClient.serverAddress === 'function') {
                        serverAddress = window.ApiClient.serverAddress();
                    }

                    api.mpris.notifyMetadata(state.NowPlayingItem, serverAddress || '');

                    // Send initial position immediately so MPRIS seekbar shows position
                    // even during buffering (don't start Playing state yet)
                    const initialPos = playbackManager.currentTime();
                    if (initialPos !== undefined && initialPos !== null) {
                        console.log('MPRIS: Initial position on playbackstart:', initialPos, 'ms');
                        api.mpris.notifyPosition(Math.floor(initialPos));
                        lastReportedPosition = initialPos;
                    }

                    // Send initial volume
                    if (player && typeof player.getVolume === 'function') {
                        const volume = player.getVolume();
                        api.mpris.notifyVolumeChange(volume / 100.0);
                    }
                }

                // Only attach global listeners once (fullscreen, queue, playback stop)
                if (!globalListenersAttached) {
                    console.log('MPRIS: Attaching global listeners (fullscreen, queue, playback stop)');

                // Listen for fullscreen setting changes and trigger fullscreenchange event
                let lastFullscreenState = window.jmpInfo.settings.main.fullscreen;
                window.jmpInfo.settingsUpdate.push(function(section) {
                    if (section === 'main') {
                        const currentFullscreenState = window.jmpInfo.settings.main.fullscreen;
                        if (currentFullscreenState !== lastFullscreenState) {
                            lastFullscreenState = currentFullscreenState;
                            console.log('MPRIS: Fullscreen setting changed, triggering fullscreenchange event');
                            const currentPlayer = playbackManager._currentPlayer;
                            if (currentPlayer) {
                                window.Events.trigger(currentPlayer, 'fullscreenchange');
                            }
                        }
                    }
                });

                // Listen for playback rate changes from native player (MPRIS)
                if (window.api && window.api.player) {
                    window.api.player.playbackRateChanged.connect(function(rate) {
                        console.log('MPRIS: Native playback rate changed to:', rate);
                        const currentPlayer = playbackManager._currentPlayer;
                        console.log('MPRIS: Current player:', currentPlayer);
                        console.log('MPRIS: Player _playRate before:', currentPlayer?._playRate);

                        // Update the current player's rate
                        if (currentPlayer && currentPlayer._playRate !== undefined) {
                            currentPlayer._playRate = rate;
                            console.log('MPRIS: Player _playRate after:', currentPlayer._playRate);
                        }

                        // Trigger ratechange event to update UI
                        if (window.Events && currentPlayer) {
                            console.log('MPRIS: Triggering ratechange event');
                            window.Events.trigger(currentPlayer, 'ratechange');
                        }
                    });
                } else {
                    console.log('MPRIS: Cannot attach playbackRateChanged listener - api:', !!window.api, 'player:', !!window.api?.player);
                }

                // Listen for volume changes from native player (MPRIS)
                if (window.api && window.api.mpris) {
                    window.api.mpris.volumeChangeRequested.connect(function(volume) {
                        console.log('MPRIS: Volume change requested:', volume);
                        const currentPlayer = playbackManager._currentPlayer;
                        if (currentPlayer && typeof currentPlayer.setVolume === 'function') {
                            currentPlayer.setVolume(volume);
                            // Volume confirmation will come via volumechange event
                        }
                    });

                    // WORKAROUND: MPRIS spec has no "Buffering" playback state
                    // During seek buffering, we set Rate to 0.0 to stop MPRIS clients from
                    // auto-incrementing position. Once buffering completes, we restore Rate to 1.0.

                    window.api.mpris.seekRequested.connect(function(positionMs) {
                        console.log('MPRIS: Seek requested to:', positionMs, 'ms');

                        const currentPlayer = playbackManager._currentPlayer;
                        if (currentPlayer) {
                            // Calculate percentage of total duration
                            const duration = playbackManager.duration();
                            if (duration) {
                                const percent = (positionMs * 10000) / duration * 100;
                                // Use seekPercent which is what the UI uses - updates UI immediately
                                playbackManager.seekPercent(percent, currentPlayer);
                            }
                        }

                        // Update MPRIS position
                        api.mpris.notifyPosition(Math.floor(positionMs));
                        lastReportedPosition = positionMs;

                        // Set Rate to 0 during buffering - will be restored when 'playing' event fires
                        api.mpris.notifyRateChange(0.0);
                        console.log('MPRIS: Set rate to 0 during seek buffering');
                    });
                }

                // Listen for queue changes (playbackManager listeners - only once)
                window.Events.on(playbackManager, 'playlistitemremove', updateQueueState);
                window.Events.on(playbackManager, 'playlistitemadd', updateQueueState);
                window.Events.on(playbackManager, 'playlistitemchange', updateQueueState);

                // Listen for playback stop to determine if exiting or navigating
                // This must come BEFORE player.playbackstop to set the navigation flag first
                window.Events.on(playbackManager, 'playbackstop', function(e, playbackStopInfo) {
                    // Update queue state on stop
                    updateQueueState();

                    // Notify if this is a full stop (back button, end of queue) vs navigation
                    const isNavigating = !!(playbackStopInfo && playbackStopInfo.nextMediaType);
                    console.log('MPRIS: Playback stopped, isNavigating:', isNavigating, 'nextMediaType:', playbackStopInfo?.nextMediaType);
                    api.mpris.notifyPlaybackStop(isNavigating);
                });

                    globalListenersAttached = true;
                }

                // Monitor duration changes and notify MPRIS (shared across player instances)
                let lastDuration = 0;
                const checkDuration = function() {
                    const duration = playbackManager.duration();
                    if (duration && duration !== lastDuration) {
                        lastDuration = duration;
                        const durationMs = Math.floor(duration / 10000); // Convert ticks to ms
                        console.log('MPRIS: Duration changed to:', durationMs, 'ms');
                        api.mpris.notifyDurationChange(durationMs);
                    }
                };

                // Only attach per-player listeners once per player instance
                console.log('MPRIS: Checking player attachment - current player:', player, 'attached player:', this.attachedPlayer, 'same?', player === this.attachedPlayer);
                if (player !== this.attachedPlayer) {
                    // Remove listeners from old player before attaching to new one
                    if (this.attachedPlayer) {
                        console.log('MPRIS: Removing listeners from old player');
                        window.Events.off(this.attachedPlayer, 'shufflequeuemodechange');
                        window.Events.off(this.attachedPlayer, 'repeatmodechange');
                        window.Events.off(this.attachedPlayer, 'playing');
                        window.Events.off(this.attachedPlayer, 'pause');
                        window.Events.off(this.attachedPlayer, 'playbackstop');
                        window.Events.off(this.attachedPlayer, 'volumechange');
                        window.Events.off(this.attachedPlayer, 'timeupdate');
                    }

                    this.attachedPlayer = player;
                    console.log('MPRIS: Attaching per-player listeners (playing, pause, stop, volume, shuffle, repeat)');

                    window.Events.on(player, 'shufflequeuemodechange', () => {
                    const mode = playbackManager.getQueueShuffleMode();
                    const enabled = (mode === 'Shuffle');
                    console.log('MPRIS: Shuffle mode changed to:', mode, 'enabled:', enabled);
                    api.mpris.notifyShuffleChange(enabled);
                });

                window.Events.on(player, 'repeatmodechange', () => {
                    const mode = playbackManager.getRepeatMode();
                    console.log('MPRIS: Repeat mode changed to:', mode);
                    api.mpris.notifyRepeatChange(mode);
                });

                window.Events.on(player, 'playing', () => {
                    console.log('MPRIS: Player playing event');
                    api.mpris.notifyPlaybackState('Playing');

                    // Update queue state now that playback is actually playing
                    updateQueueState();

                    // Restore rate to 1.0 since playback started (handles seek buffering)
                    api.mpris.notifyRateChange(1.0);
                    console.log('MPRIS: Restored rate to 1.0 on playing event');

                    // Clear any existing intervals and restart (handles multiple player instances)
                    if (this.durationCheckInterval) {
                        clearInterval(this.durationCheckInterval);
                    }
                    checkDuration();
                    this.durationCheckInterval = setInterval(checkDuration, 1000);

                    // Send initial position immediately
                    const initialPos = playbackManager.currentTime();
                    if (initialPos !== undefined && initialPos !== null) {
                        console.log('MPRIS: Initial position:', initialPos, 'ms');
                        api.mpris.notifyPosition(Math.floor(initialPos));
                    }

                    // Clear any existing position interval and restart
                    if (this.positionUpdateInterval) {
                        clearInterval(this.positionUpdateInterval);
                    }
                    this.positionUpdateInterval = setInterval(() => {
                        const positionMs = playbackManager.currentTime();
                        if (positionMs !== undefined && positionMs !== null) {
                            // Detect seeks (position jump > 2 seconds)
                            const positionDiff = Math.abs(positionMs - lastReportedPosition);
                            if (lastReportedPosition > 0 && positionDiff > 2000) {
                                console.log('MPRIS: Position jump detected (player seek):', lastReportedPosition, '->', positionMs);

                                // Player-initiated seek detected - set rate to 0 during buffering
                                // Rate will be restored to 1.0 when 'playing' event fires
                                api.mpris.notifyRateChange(0.0);
                                console.log('MPRIS: Set rate to 0 for player-initiated seek');

                                // Notify as a seek to trigger Seeked signal
                                api.mpris.notifySeek(Math.floor(positionMs));
                            } else {
                                api.mpris.notifyPosition(Math.floor(positionMs));
                            }
                            lastReportedPosition = positionMs;
                        }
                    }, 500);
                });

                window.Events.on(player, 'pause', () => {
                    console.log('MPRIS: Player pause event');
                    api.mpris.notifyPlaybackState('Paused');
                    api.mpris.notifyRateChange(0.0);

                    if (this.durationCheckInterval) {
                        clearInterval(this.durationCheckInterval);
                        this.durationCheckInterval = null;
                    }

                    // Don't stop position updates - we need to detect seeks while paused
                    // Just update lastReportedPosition to current position
                    const currentPos = playbackManager.currentTime();
                    if (currentPos !== undefined && currentPos !== null) {
                        lastReportedPosition = currentPos;
                    }
                });

                window.Events.on(player, 'playbackstop', () => {
                    console.log('MPRIS: Player stop event - cleanup only');
                    // Don't call notifyPlaybackState here - it's handled by notifyPlaybackStop
                    // which has the navigation flag information

                    if (this.durationCheckInterval) {
                        clearInterval(this.durationCheckInterval);
                        this.durationCheckInterval = null;
                    }

                    if (this.positionUpdateInterval) {
                        clearInterval(this.positionUpdateInterval);
                        this.positionUpdateInterval = null;
                    }

                    lastDuration = 0;
                    lastReportedPosition = 0;
                });

                    window.Events.on(player, 'volumechange', () => {
                        if (player && typeof player.getVolume === 'function') {
                            const volume = player.getVolume();
                            console.log('MPRIS: Volume changed to:', volume);
                            // Notify MPRIS (convert from 0-100 to 0.0-1.0)
                            api.mpris.notifyVolumeChange(volume / 100.0);
                        }
                    });

                    // Listen for position updates (including seeks while paused)
                    window.Events.on(player, 'timeupdate', () => {
                        const positionMs = playbackManager.currentTime();
                        if (positionMs !== undefined && positionMs !== null) {
                            // Detect seeks (position jump > 2 seconds)
                            const positionDiff = Math.abs(positionMs - lastReportedPosition);
                            if (lastReportedPosition > 0 && positionDiff > 2000) {
                                console.log('MPRIS: Position jump detected via timeupdate:', lastReportedPosition, '->', positionMs);

                                // Player-initiated seek - set rate to 0 during buffering
                                api.mpris.notifyRateChange(0.0);
                                console.log('MPRIS: Set rate to 0 for player-initiated seek (timeupdate)');

                                api.mpris.notifySeek(Math.floor(positionMs));
                            } else if (positionDiff > 100) {
                                // Small position changes (like seeks while paused)
                                api.mpris.notifyPosition(Math.floor(positionMs));
                            }
                            lastReportedPosition = positionMs;
                        }
                    });
                }
            });

            api.system.hello("jmpInputPlugin");
        })();
    }

    destroy() {
        if (this.durationCheckInterval) {
            clearInterval(this.durationCheckInterval);
            this.durationCheckInterval = null;
        }

        if (this.positionUpdateInterval) {
            clearInterval(this.positionUpdateInterval);
            this.positionUpdateInterval = null;
        }

        if (this.attachedPlayer) {
            window.Events.off(this.attachedPlayer, 'shufflequeuemodechange');
            window.Events.off(this.attachedPlayer, 'repeatmodechange');
            window.Events.off(this.attachedPlayer, 'playing');
            window.Events.off(this.attachedPlayer, 'pause');
            window.Events.off(this.attachedPlayer, 'playbackstop');
            window.Events.off(this.attachedPlayer, 'volumechange');
            window.Events.off(this.attachedPlayer, 'timeupdate');
            this.attachedPlayer = null;
        }

        if (window.playbackManager) {
            window.Events.off(window.playbackManager, 'playlistitemremove');
            window.Events.off(window.playbackManager, 'playlistitemadd');
            window.Events.off(window.playbackManager, 'playlistitemchange');
            window.Events.off(window.playbackManager, 'playbackstop');
        }

        console.log('MPRIS: jmpInputPlugin destroyed and cleaned up');
    }
}

window._jmpInputPlugin = jmpInputPlugin;