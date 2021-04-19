class jmpUpdatePlugin {
    constructor({ confirm, toast }) {
        this.name = 'JMP Update Plugin';
        this.type = 'input';
        this.id = 'jmpUpdatePlugin';

        (async () => {
            const api = await window.apiPromise;

            const onUpdateNotify = async (url) => {
                const urlSegments = url.split("/");
                const version = urlSegments[urlSegments.length - 1].substring(1);
                const currentVersion = navigator.userAgent.split(" ")[1];

                if (version == currentVersion) return;
                if (!/^[0-9.-]+$/.test(version)) return;

                try {
                    await confirm({
                        title: "Update Available",
                        text: `Jellyfin Media Player version ${version} is available.`,
                        cancelText: "Ignore",
                        confirmText: "Download"
                    });

                    api.system.openExternalUrl(url);
                } catch (e) {
                    // User cancelled update
                }
            }

            api.system.updateInfoEmitted.connect(onUpdateNotify);
            api.system.checkForUpdates();
        })();
    }
}

window._jmpUpdatePlugin = jmpUpdatePlugin;