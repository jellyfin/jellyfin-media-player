let tvIntro;

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
                    Skip Intro
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

            async function injectSkipIntroHtml() {
                const playerContainer = await waitForElement('.upNextContainer', 5000);
                // inject only if it doesn't exist
                if (!document.querySelector('.skipIntro .btnSkipIntro')) {
                    playerContainer.insertAdjacentHTML('afterend', skipIntroHtml);
                }

                document.querySelector('.skipIntro .btnSkipIntro').addEventListener('click', (e) => {
                    e.preventDefault();
                    e.stopPropagation();
                    skipIntro();
                }, { useCapture: true });

                if (window.PointerEvent) {
                    document.querySelector('.skipIntro .btnSkipIntro').addEventListener('pointerdown', (e) => {
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
                        if (!tvIntro?.Valid) {
                            return;
                        }

                        const seconds = playbackManager.currentTime(player) / 1000;

                        await injectSkipIntroHtml(); // I have trust issues
                        const skipIntro = document.querySelector(".skipIntro");
    
                        // If the skip prompt should be shown, show it.
                        if (seconds >= tvIntro.ShowSkipPromptAt && seconds < tvIntro.HideSkipPromptAt) {
                            skipIntro.classList.remove("hide");
                            return;
                        }

                        skipIntro.classList.add("hide");
                    };

                    events.on(player, 'timeupdate', onTimeUpdate);

                    const onPlaybackStop = () => {
                        events.off(player, 'timeupdate', onTimeUpdate);
                        events.off(player, 'playbackstop', onPlaybackStop);
                    };
                    events.on(player, 'playbackstop', onPlaybackStop);
                }
            };
            events.on(playbackManager, 'playbackstart', onPlayback);
            

            function getIntroTimestamps(item) {
                const apiClient = ServerConnections
                    ? ServerConnections.currentApiClient()
                    : window.ApiClient;
                const address = apiClient.serverAddress();

                const url = `${address}/Episode/${item.Id}/IntroTimestamps`;
                const reqInit = {
                    headers: {
                        "Authorization": `MediaBrowser Token=${apiClient.accessToken()}`
                    }
                };

                fetch(url, reqInit).then(r => {
                    if (!r.ok) {
                        tvIntro = null;
                        return;
                    }

                    return r.json();
                }).then(intro => {
                    tvIntro = intro;
                }).catch(err => { tvIntro = null; });
            }

            function skipIntro() {
                playbackManager.seekMs(tvIntro.IntroEnd * 1000);
            }
        })();
    }
}

window._skipIntroPlugin = skipIntroPlugin;