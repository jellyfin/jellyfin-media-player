async function tryConnect(server) {
    document.getElementById('connect-button').disabled = true;

    try {
        if (!server.startsWith("http")) {
            server = "http://" + server;
        }

        const url = new URL("/System/Info/Public", server);
        const response = await fetch(url);
        if (response.ok && (await response.json()).Id) {
            const htmlResponse = await fetch(server);
            if (!htmlResponse.ok) {
                throw new Error("Status not ok");
            }

            // Sigh... If we just navigate to the URL, the server's CSP will block us loading other resources.
            // So we have to parse the HTML, set a new base href, and then write it back to the page.
            // We also have to override the history functions to make sure they use the correct URL.
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
                url = (new URL(url, realUrl)).toString();
                return oldPushState.call(window.history, state, title, url);
            };

            const oldReplaceState = window.history.replaceState;
            window.history.replaceState = function(state, title, url) {
                url = (new URL(url, realUrl)).toString();
                return oldReplaceState.call(window.history, state, title, url);
            };

            document.open();
            document.write((new XMLSerializer()).serializeToString(doc));
            document.close();

            window.localStorage.setItem("saved-server", server);
            return true;
        }
    } catch (e) {
        console.error(e);
        document.getElementById('connect-button').disabled = false;
        return false;
    }
}

document.getElementById('connect-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    const server = document.getElementById('address').value;
    const result = await tryConnect(server);
    if (!result) {
        document.getElementById('backdrop').style.display = 'block';
    }
});

document.getElementById('connect-fail-button').addEventListener('click', () => {
    document.getElementById('backdrop').style.display = 'none';
});

const savedServer = window.localStorage.getItem("saved-server");

// load the server if we have one
(async() => {
    if (!savedServer || !(await tryConnect(savedServer))) {
        document.getElementById('splash').style.display = 'none';
        document.getElementById('main').style.display = 'block';
    }
})();
