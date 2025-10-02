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

        (async () => {
            const api = await window.apiPromise;

            api.input.hostInput.connect((actions) => {
                actions.forEach(action => {
                    // Handle shuffle/sorted actions directly
                    if (action === 'shuffle') {
                        console.log('MPRIS: Setting shuffle mode to Shuffle');
                        playbackManager.setQueueShuffleMode('Shuffle');
                        return;
                    } else if (action === 'sorted') {
                        console.log('MPRIS: Setting shuffle mode to Sorted');
                        playbackManager.setQueueShuffleMode('Sorted');
                        return;
                    }

                    if (remap.hasOwnProperty(action)) {
                        action = remap[action];
                    }
                    inputManager.handleCommand(action, {});
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
                    const canPrevious = currentIndex > 0;

                    api.mpris.notifyQueueChange(canNext, canPrevious);
                } catch (e) {
                    // Silently fail
                }
            };

            // Listen for shuffle and repeat mode changes and notify native player
            // Attach listeners on playbackstart event
            let listenersAttached = false;
            window.Events.on(playbackManager, 'playbackstart', (e, player) => {
                if (!player) return;

                // Always update queue state on playback start
                updateQueueState();

                // Only attach player-specific listeners once
                if (listenersAttached) return;

                console.log('MPRIS: Attaching shuffle, repeat, and fullscreen listeners to player');
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

                // Listen for fullscreen setting changes and trigger fullscreenchange event
                let lastFullscreenState = window.jmpInfo.settings.main.fullscreen;
                window.jmpInfo.settingsUpdate.push(function(section) {
                    if (section === 'main') {
                        const currentFullscreenState = window.jmpInfo.settings.main.fullscreen;
                        if (currentFullscreenState !== lastFullscreenState) {
                            lastFullscreenState = currentFullscreenState;
                            console.log('MPRIS: Fullscreen setting changed, triggering fullscreenchange event');
                            window.Events.trigger(player, 'fullscreenchange');
                        }
                    }
                });

                // Listen for playback rate changes from native player (MPRIS)
                if (window.api && window.api.player) {
                    window.api.player.playbackRateChanged.connect(function(rate) {
                        console.log('MPRIS: Native playback rate changed to:', rate);
                        console.log('MPRIS: Current player:', player);
                        console.log('MPRIS: Player _playRate before:', player?._playRate);

                        // Update the current player's rate
                        if (player && player._playRate !== undefined) {
                            player._playRate = rate;
                            console.log('MPRIS: Player _playRate after:', player._playRate);
                        }

                        // Trigger ratechange event to update UI
                        if (window.Events) {
                            console.log('MPRIS: Triggering ratechange event');
                            window.Events.trigger(player, 'ratechange');
                        }
                    });
                } else {
                    console.log('MPRIS: Cannot attach playbackRateChanged listener - api:', !!window.api, 'player:', !!window.api?.player);
                }

                // Listen for queue changes
                window.Events.on(playbackManager, 'playlistitemremove', updateQueueState);
                window.Events.on(playbackManager, 'playlistitemadd', updateQueueState);
                window.Events.on(playbackManager, 'playlistitemchange', updateQueueState);

                listenersAttached = true;
            });

            api.system.hello("jmpInputPlugin");
        })();
    }
}

window._jmpInputPlugin = jmpInputPlugin;