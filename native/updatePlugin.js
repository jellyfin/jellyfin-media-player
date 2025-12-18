(function() {
class updatePlugin {
    constructor({ confirm }) {
        this.name = 'Update Plugin';
        this.type = 'input';
        this.id = 'updatePlugin';

        (async () => {
            const api = await window.apiPromise;

            const onUpdateNotify = async (url) => {
                if (url == "SSL_UNAVAILABLE") {
                    // Windows (and possibly macOS) don't ship with SSL in QT......
                    // So we get to do a full request to GitHub here :(
                    const checkUrl = "https://github.com/jellyfin/jellyfin-desktop/releases/latest";
                    url = (await fetch(checkUrl)).url;
                }

                const urlSegments = url.split("/");
                const version = urlSegments[urlSegments.length - 1].substring(1);
                const currentVersion = jmpInfo.version;

                if (currentVersion.includes('pre')) return; // Do not notify for prereleases
                if (version == currentVersion) return;
                if (!/^[0-9.-]+$/.test(version)) return;

                try {
                    // wait 3 seconds before showing the dialog to prevent race conditions
                    await new Promise(resolve => setTimeout(resolve, 3000));

                    await confirm({
                        title: "Update Available",
                        text: `Jellyfin Desktop version ${version} is available.`,
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

window._updatePlugin = updatePlugin;
})();
