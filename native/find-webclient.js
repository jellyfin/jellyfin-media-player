async function tryConnect(server) {
    try {
        if (!server.startsWith("http")) {
            server = "http://" + server;
        }
        serverBaseURL = server.replace(/\/+$/, "");
        const url = serverBaseURL + "/System/Info/Public";

        const controller = new AbortController();
        const timeoutId = setTimeout(() => controller.abort(), 60000);

        const response = await fetch(url, { cache: 'no-cache', signal: controller.signal });
        clearTimeout(timeoutId);
        if (response.ok && (await response.json()).Id) {
            const htmlResponse = await fetch(server, { cache: 'no-cache' });
            if (!htmlResponse.ok) {
                throw new Error("Status not ok");
            }

            if (response.headers.get("content-security-policy")) {
                console.log("Using CSP workaround");
                const webUrl = htmlResponse.url.replace(/\/[^\/]*$/, "/");
                const realUrl = window.location.href;

                const html = await htmlResponse.text();
                const parser = new DOMParser();
                const doc = parser.parseFromString(html, "text/html");
                const base = doc.createElement("base");
                base.href = webUrl
                doc.head.insertBefore(base, doc.head.firstChild);

                const oldPushState = window.history.pushState;
                window.history.pushState = function(state, title, url) {
                    url = url ? (new URL(url, realUrl)).toString() : realUrl;
                    return oldPushState.call(window.history, state, title, url);
                };

                const oldReplaceState = window.history.replaceState;
                window.history.replaceState = function(state, title, url) {
                    url = url ? (new URL(url, realUrl)).toString() : realUrl;
                    return oldReplaceState.call(window.history, state, title, url);
                };

                document.open();
                document.write((new XMLSerializer()).serializeToString(doc));
                document.close();
            } else {
                console.log("Using normal navigation");
                window.location = server;
            }

            await window.initCompleted;
            window.jmpInfo.settings.main.userWebClient = server;

            return true;
        }
        return false;
    } catch (e) {
        console.error(e);
        return false;
    }
}

let isConnecting = false;

const updateButtonState = () => {
    const address = document.getElementById('address');
    const button = document.getElementById('connect-button');
    const hasValue = address.value.trim().length > 0;

    if (!isConnecting) {
        button.disabled = !hasValue;
    }
};

const cancelOnEscape = (e) => {
    if (isConnecting && e.key === 'Escape') {
        cancelConnection();
    }
};

const startConnecting = async () => {
    const address = document.getElementById('address');
    const title = document.getElementById('title');
    const spinner = document.getElementById('spinner');
    const button = document.getElementById('connect-button');
    const server = address.value;

    isConnecting = true;
    title.textContent = '';
    title.style.visibility = 'hidden';
    address.classList.add('connecting');
    address.style.visibility = 'hidden';
    address.disabled = true;
    spinner.style.display = 'block';
    button.style.visibility = 'hidden';
    document.addEventListener('keydown', cancelOnEscape);

    let connected = false;

    while (!connected && isConnecting) {
        connected = await tryConnect(server);

        if (!connected && isConnecting) {
            // Wait 5 seconds before retrying
            await new Promise(resolve => setTimeout(resolve, 5000));
        }
    }

    if (!connected) {
        isConnecting = false;
        title.textContent = document.getElementById('title').getAttribute('data-original-text');
        title.style.visibility = 'visible';
        address.classList.remove('connecting');
        address.style.visibility = 'visible';
        address.disabled = false;
        spinner.style.display = 'none';
        button.style.visibility = 'visible';
        document.removeEventListener('keydown', cancelOnEscape);
        updateButtonState();
    }
};

const cancelConnection = () => {
    if (!isConnecting) return;

    isConnecting = false;

    const address = document.getElementById('address');
    const title = document.getElementById('title');
    const spinner = document.getElementById('spinner');
    const button = document.getElementById('connect-button');

    title.textContent = document.getElementById('title').getAttribute('data-original-text');
    title.style.visibility = 'visible';
    address.classList.remove('connecting');
    address.style.visibility = 'visible';
    address.disabled = false;
    spinner.style.display = 'none';
    button.style.visibility = 'visible';
    document.removeEventListener('keydown', cancelOnEscape);
    updateButtonState();
};

// Button click handler
document.getElementById('connect-button').addEventListener('click', (e) => {
    e.preventDefault();
    e.stopPropagation();

    if (!e.target.disabled) {
        startConnecting();
    }
});

// Form submit handler
document.getElementById('connect-form').addEventListener('submit', (e) => {
    e.preventDefault();
    if (!isConnecting) {
        startConnecting();
    }
});

// Input change handler
document.getElementById('address').addEventListener('input', updateButtonState);


// Enter key handler
document.addEventListener('keydown', (e) => {
    const address = document.getElementById('address');
    if (e.key === 'Enter' && !isConnecting && !address.disabled && address.value.trim()) {
        e.preventDefault();
        startConnecting();
    }
});

// Auto-connect on load
(async() => {
    const title = document.getElementById('title');
    const address = document.getElementById('address');
    const button = document.getElementById('connect-button');

    const savedServer = window.jmpInfo.settings.main.userWebClient;
    if (savedServer) {
        address.value = savedServer;
        await startConnecting();
    } else {
        title.style.visibility = 'visible';
        address.style.visibility = 'visible';
        button.style.visibility = 'visible';
        address.focus();
        updateButtonState();
    }
})();
