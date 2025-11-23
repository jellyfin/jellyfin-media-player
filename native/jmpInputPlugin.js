(function() {
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
                    if (remap.hasOwnProperty(action)) {
                        action = remap[action];
                    }
                    inputManager.handleCommand(action, {});
                });
            });

            // Listen for fullscreen setting changes and trigger fullscreenchange event
            let lastFullscreenState = window.jmpInfo.settings.main.fullscreen;
            window.jmpInfo.settingsUpdate.push(function(section) {
                if (section === 'main') {
                    const currentFullscreenState = window.jmpInfo.settings.main.fullscreen;
                    if (currentFullscreenState !== lastFullscreenState) {
                        lastFullscreenState = currentFullscreenState;
                        console.log('[jmpInputPlugin] Fullscreen setting changed, triggering fullscreenchange event');
                        const currentPlayer = playbackManager._currentPlayer;
                        if (currentPlayer) {
                            window.Events.trigger(currentPlayer, 'fullscreenchange');
                        }
                    }
                }
            });

            api.system.hello("jmpInputPlugin");
        })();
    }
}

window._jmpInputPlugin = jmpInputPlugin;
})();