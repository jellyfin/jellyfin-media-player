let tvIntro;
let currentSegment;
let currentPlayer;
let skipIntroConfig;

class skipIntroPlugin {
    constructor({ events, playbackManager, ServerConnections }) {
        this.name = 'Skip Intro Plugin';
        this.type = 'input';
        this.id = 'skipIntroPlugin';

        (async() => {
            await window.initCompleted;
            const enabled = window.jmpInfo.settings.plugins.skipintro;

            console.log("Skip Intro Plugin enabled: " + enabled);
            if (!enabled) return;

            // Based on https://github.com/jellyfin/jellyfin-web/compare/release-10.8.z...ConfusedPolarBear:jellyfin-web:intros
            // Updated from https://github.com/jumoog/intro-skipper/blob/master/ConfusedPolarBear.Plugin.IntroSkipper/Configuration/inject.js
            // Adapted for use in JMP
            const stylesheet = `
            <style>
            @media (hover:hover) and (pointer:fine) {
                .skipIntro .paper-icon-button-light:hover:not(:disabled) {
                    color:black !important;
                    background-color:rgba(47,93,98,0) !important;
                }
            }

            .skipIntro {
                padding: 0 1px;
                position: absolute;
                right: 10em;
                bottom: 9em;
                background-color:rgba(25, 25, 25, 0.66);
                border: 1px solid;
                border-radius: 0px;
                display: inline-block;
                cursor: pointer;
                box-shadow: inset 0 0 0 0 #f9f9f9;
                -webkit-transition: ease-out 0.4s;
                -moz-transition: ease-out 0.4s;
                transition: ease-out 0.4s;
            }

            @media (max-width: 1080px) {
                .skipIntro {
                    right: 10%;
                }
            }

            .skipIntro:hover {
                box-shadow: inset 400px 0 0 0 #f9f9f9;
                -webkit-transition: ease-in 1s;
                -moz-transition: ease-in 1s;
                transition: ease-in 1s;
            }
            </style>
            `;

            document.head.insertAdjacentHTML('beforeend', stylesheet);


            const skipIntroHtml = `
            <div class="skipIntro hide">
                <button is="paper-icon-button-light" class="btnSkipIntro paper-icon-button-light">
                    <span id="btnSkipSegmentText"></span>
                    <span class="material-icons skip_next"></span>
                </button>
            </div>
            `;

            function waitForElement(element, maxWait = 10000) {
                return new Promise((resolve, reject) => {
                    const interval = setInterval(() => {
                        const result = document.querySelector(element);
                        if (result) {
                            clearInterval(interval);
                            resolve(result);
                        }
                    }, 100);

                    setTimeout(() => {
                        clearInterval(interval);
                        reject();
                    }, maxWait);
                });
            }

            function handleClick(e) {
                e.preventDefault();
                e.stopPropagation();
                doSkip();
                document.querySelector('.skipIntro .btnSkipIntro').removeEventListener('click', handleClick, { useCapture: true });
            }

            function secureFetch(url) {
                const apiClient = ServerConnections
                    ? ServerConnections.currentApiClient()
                    : window.ApiClient;
                const address = apiClient.serverAddress();

                const reqInit = {
                    headers: {
                        "Authorization": `MediaBrowser Token=${apiClient.accessToken()}`
                    }
                };

                return fetch(`${address}${url}`, reqInit).then(r => {
                    if (!r.ok) {
                        tvIntro = null;
                        return;
                    }

                    return r.json();
                });
            }

            async function injectSkipIntroHtml() {
                const playerContainer = await waitForElement('.upNextContainer', 5000);
                // inject only if it doesn't exist
                if (!document.querySelector('.skipIntro .btnSkipIntro')) {
                    playerContainer.insertAdjacentHTML('afterend', skipIntroHtml);

                    skipIntroConfig = await secureFetch("/Intros/UserInterfaceConfiguration");
                    if (!skipIntroConfig.SkipButtonVisible) {
                        console.info("[intro skipper] Skip button is disabled by the server.");
                        return;
                    }
                }

                const button = document.querySelector('.skipIntro .btnSkipIntro');
                button.addEventListener('click', handleClick, { useCapture: true });

                if (window.PointerEvent) {
                    button.addEventListener('pointerdown', (e) => {
                        e.preventDefault();
                        e.stopPropagation();
                    }, { useCapture: true });
                }
            }


            function onPlayback(e, player, state) {
                if (state.NowPlayingItem) {
                    getIntroTimestamps(state.NowPlayingItem);

                    const onTimeUpdate = async () => {
                        // Check if an introduction sequence was detected for this item.
                        if (!tvIntro) {
                            return;
                        }

                        await injectSkipIntroHtml(); // I have trust issues
                        const skipIntro = document.querySelector(".skipIntro");
                        if (!skipIntro) {
                            return;
                        }

                        const segment = getCurrentSegment(playbackManager.currentTime(player) / 1000);
                        currentSegment = segment;
                        currentPlayer = player;
                        switch (segment["SegmentType"]) {
                            case "None":
                                if (skipIntro.style.opacity === '0') return;

                                skipIntro.style.opacity = '0';
                                skipIntro.addEventListener("transitionend", () => {
                                    skipIntro.classList.add("hide");
                                }, { once: true });
                                return;
                            case "Introduction":
                                skipIntro.querySelector("#btnSkipSegmentText").textContent = skipIntroConfig.SkipButtonIntroText;
                                break;
                            case "Credits":
                                skipIntro.querySelector("#btnSkipSegmentText").textContent = skipIntroConfig.SkipButtonEndCreditsText;
                                break;
                        }
                        if (!skipIntro.classList.contains("hide")) return;

                        skipIntro.classList.remove("hide");
                        skipIntro.style.display = 'block';
                        requestAnimationFrame(() => {
                            requestAnimationFrame(() => {
                                skipIntro.style.opacity = '1';
                            });
                        });
                    };

                    events.on(player, 'timeupdate', onTimeUpdate);

                    const onPlaybackStop = () => {
                        events.off(player, 'timeupdate', onTimeUpdate);
                        events.off(player, 'playbackstop', onPlaybackStop);

                        // Reset skipIntro styles on playbackstop
                        const skipIntro = document.querySelector(".skipIntro");
                        if (skipIntro) {
                            skipIntro.style.display = 'none';
                            skipIntro.style.opacity = '0';
                            skipIntro.classList.add("hide");
                        }
                    };
                    events.on(player, 'playbackstop', onPlaybackStop);
                }
            };
            events.on(playbackManager, 'playbackstart', onPlayback);

            function getIntroTimestamps(item) {
                secureFetch(`/Episode/${item.Id}/IntroSkipperSegments`).then(intro => {
                    tvIntro = intro;
                }).catch(err => { tvIntro = null; });
            }

            function osdVisible() {
                const osd = document.querySelector("div.videoOsdBottom");
                return osd ? !osd.classList.contains("hide") : false;
            }

            function getCurrentSegment(position) {
                for (let key in tvIntro) {
                    const segment = tvIntro[key];
                    if ((position >= segment.ShowSkipPromptAt && position < segment.HideSkipPromptAt) || (osdVisible() && position >= segment.IntroStart && position < segment.IntroEnd)) {
                        segment["SegmentType"] = key;
                        return segment;
                    }
                }
                return { "SegmentType": "None" };
            }

            function doSkip(e) {
                if (currentSegment["SegmentType"] === "None") {
                    console.warn("[intro skipper] doSkip() called without an active segment");
                    return;
                }
                currentPlayer.currentTime(currentSegment["IntroEnd"] * 1000);
            }

        })();
    }
}

window._skipIntroPlugin = skipIntroPlugin;
