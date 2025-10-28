window.jmpCheckServerConnectivity = (() => {
    let checkInProgress = false;

    return async function(url) {
        if (checkInProgress) {
            throw new Error('Connectivity check already in progress');
        }

        // Wait for API
        let attempts = 0;
        while (!window.api && attempts < 50) {
            await new Promise(resolve => setTimeout(resolve, 100));
            attempts++;
        }
        if (!window.api) {
            throw new Error('WebChannel not available');
        }

        checkInProgress = true;

        return new Promise((resolve, reject) => {
            const handler = (resultUrl, success) => {
                if (resultUrl === url) {
                    window.api.system.serverConnectivityResult.disconnect(handler);
                    checkInProgress = false;
                    if (success) {
                        resolve();
                    } else {
                        reject(new Error('Connection failed'));
                    }
                }
            };
            window.api.system.serverConnectivityResult.connect(handler);
            window.api.system.checkServerConnectivity(url);
        });
    };
})();

window.jmpFetchPage = (() => {
    let fetchInProgress = false;

    return async function(url) {
        if (fetchInProgress) {
            throw new Error('Page fetch already in progress');
        }

        // Wait for API
        let attempts = 0;
        while (!window.api && attempts < 50) {
            await new Promise(resolve => setTimeout(resolve, 100));
            attempts++;
        }
        if (!window.api) {
            throw new Error('WebChannel not available');
        }

        fetchInProgress = true;

        return new Promise((resolve, reject) => {
            const handler = (html, finalUrl, hadCSP) => {
                window.api.system.pageContentReady.disconnect(handler);
                fetchInProgress = false;
                resolve({ html, finalUrl, hadCSP });
            };
            window.api.system.pageContentReady.connect(handler);
            window.api.system.fetchPageForCSPWorkaround(url);
        });
    };
})();
