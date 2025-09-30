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

            // Listen for shuffle and repeat mode changes and notify native player
            // Attach listeners on playbackstart event
            let listenersAttached = false;
            window.Events.on(playbackManager, 'playbackstart', (e, player) => {
                if (listenersAttached || !player) return;

                console.log('MPRIS: Attaching shuffle and repeat listeners to player');
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
                listenersAttached = true;
            });

            api.system.hello("jmpInputPlugin");
        })();
    }
}

window._jmpInputPlugin = jmpInputPlugin;