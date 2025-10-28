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
    constructor({ inputManager }) {
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

            api.system.hello("jmpInputPlugin");
        })();
    }
}

window._jmpInputPlugin = jmpInputPlugin;
})();