const viewdata = JSON.parse(window.atob("@@data@@"));

const features = [
    "filedownload",
    "displaylanguage",
    "htmlaudioautoplay",
    "htmlvideoautoplay",
    "externallinks",
    "clientsettings",
    "multiserver",
    "remotecontrol",
    "fullscreenchange",
    "filedownload",
    "remotevideo",
    "displaymode",
    "screensaver",
    "fileinput"
];

const plugins = [
    'mpvVideoPlayer',
    'mpvAudioPlayer',
    'jmpInputPlugin',
    'jmpUpdatePlugin'
];

function loadScript(src) {
    return new Promise((resolve, reject) => {
        const s = document.createElement('script');
        s.src = src;
        s.onload = resolve;
        s.onerror = reject;
        document.head.appendChild(s);
    });
}

// Add plugin loaders
for (const plugin of plugins) {
    window[plugin] = async () => {
        await loadScript(`${viewdata.scriptPath}${plugin}.js`);
        return window["_" + plugin];
    };
}

window.NativeShell = {
    openUrl(url, target) {
        window.api.system.openExternalUrl(url);
    },

    downloadFile(downloadInfo) {
        window.api.system.openExternalUrl(downloadInfo.url);
    },

    openClientSettings() {
        showSettingsModal();
    },

    getPlugins() {
        return plugins;
    }
};

function getDeviceProfile() {
    return {
        'Name': 'Jellyfin Media Player',
        'MusicStreamingTranscodingBitrate': 1280000,
        'TimelineOffsetSeconds': 5,
        'TranscodingProfiles': [
            {'Type': 'Audio'},
            {
                'Container': 'ts',
                'Type': 'Video',
                'Protocol': 'hls',
                'AudioCodec': 'aac,mp3,ac3,opus,flac,vorbis',
                'VideoCodec': 'h264,h265,hevc,mpeg4,mpeg2video',
                'MaxAudioChannels': '6'
            },
            {'Container': 'jpeg', 'Type': 'Photo'}
        ],
        'DirectPlayProfiles': [{'Type': 'Video'}, {'Type': 'Audio'}, {'Type': 'Photo'}],
        'ResponseProfiles': [],
        'ContainerProfiles': [],
        'CodecProfiles': [],
        'SubtitleProfiles': [
            {'Format': 'srt', 'Method': 'External'},
            {'Format': 'srt', 'Method': 'Embed'},
            {'Format': 'ass', 'Method': 'External'},
            {'Format': 'ass', 'Method': 'Embed'},
            {'Format': 'sub', 'Method': 'Embed'},
            {'Format': 'sub', 'Method': 'External'},
            {'Format': 'ssa', 'Method': 'Embed'},
            {'Format': 'ssa', 'Method': 'External'},
            {'Format': 'smi', 'Method': 'Embed'},
            {'Format': 'smi', 'Method': 'External'},
            {'Format': 'pgssub', 'Method': 'Embed'},
            {'Format': 'dvdsub', 'Method': 'Embed'},
            {'Format': 'pgs', 'Method': 'Embed'}
        ]
    };
}

async function createApi() {
    await loadScript('qrc:///qtwebchannel/qwebchannel.js');
    const channel = await new Promise((resolve) => {
        /*global QWebChannel */
        new QWebChannel(window.qt.webChannelTransport, resolve);
    });
    return channel.objects;
}

window.NativeShell.AppHost = {
    init() {
        window.apiPromise = createApi();
        (async () => {
            window.api = await window.apiPromise;
        })();
    },
    getDefaultLayout() {
        return viewdata.mode;
    },
    supports(command) {
        return features.includes(command.toLowerCase());
    },
    getDeviceProfile,
    getSyncProfile: getDeviceProfile,
    appName() {
        return "Jellyfin Media Player";
    },
    appVersion() {
        return navigator.userAgent.split(" ")[1];
    },
    deviceName() {
        return viewdata.deviceName;
    }
};

async function showSettingsModal() {
    let settings = await new Promise(resolve => {
        window.api.settings.settingDescriptions(resolve);
    });

    const modalContainer = document.createElement("div");
    modalContainer.className = "dialogContainer";
    modalContainer.style.backgroundColor = "rgba(0,0,0,0.5)";
    modalContainer.addEventListener("click", e => {
        if (e.target == modalContainer) {
            modalContainer.remove();
        }
    });
    document.body.appendChild(modalContainer);

    const modalContainer2 = document.createElement("div");
    modalContainer2.className = "focuscontainer dialog dialog-fixedSize dialog-small formDialog opened";
    modalContainer.appendChild(modalContainer2);

    const modalHeader = document.createElement("div");
    modalHeader.className = "formDialogHeader";
    modalContainer2.appendChild(modalHeader);

    const title = document.createElement("h3");
    title.className = "formDialogHeaderTitle";
    title.textContent = "Jellyfin Media Player Settings";
    modalHeader.appendChild(title);
    
    const modalContents = document.createElement("div");
    modalContents.className = "formDialogContent smoothScrollY";
    modalContents.style.paddingTop = "2em";
    modalContainer2.appendChild(modalContents);
    
    for (let section of settings) {
        const group = document.createElement("fieldset");
        group.className = "editItemMetadataForm editMetadataForm dialog-content-centered";
        group.style.border = 0;
        group.style.outline = 0;
        modalContents.appendChild(group);

        const createSection = async (clear) => {
            if (clear) {
                group.innerHTML = "";
            }

            const values = await new Promise(resolve => {
                window.api.settings.allValues(section.key, resolve);
            });

            const legend = document.createElement("legend");
            legend.innerHTML = "<h2>" + section.key + "</h2>";
            legend.style.textTransform = "capitalize";
            group.appendChild(legend);

            for (const setting of section.settings) {
                const label = document.createElement("label");
                label.className = "inputContainer";
                label.style.marginBottom = "1.8em";
                label.style.display = "block";
                label.style.textTransform = "capitalize";
                if (setting.options) {
                    const safeValues = {};
                    const control = document.createElement("select");
                    control.className = "emby-select-withcolor emby-select";
                    for (const option of setting.options) {
                        safeValues[String(option.value)] = option.value;
                        const opt = document.createElement("option");
                        opt.value = option.value;
                        opt.selected = option.value == values[setting.key];
                        let optionName = option.title;
                        const swTest = `${section.key}.${setting.key}.`;
                        const swTest2 = `${section.key}.`;
                        if (optionName.startsWith(swTest)) {
                            optionName = optionName.substring(swTest.length);
                        } else if (optionName.startsWith(swTest2)) {
                            optionName = optionName.substring(swTest2.length);
                        }
                        opt.appendChild(document.createTextNode(optionName));
                        control.appendChild(opt);
                    }
                    control.addEventListener("change", async (e) => {
                        await new Promise(resolve => {
                            window.api.settings.setValue(section.key, setting.key, safeValues[e.target.value], resolve);
                        });

                        if (setting.key == "devicetype") {
                            section = (await new Promise(resolve => {
                                window.api.settings.settingDescriptions(resolve);
                            })).filter(x => x.key == section.key)[0];
                            createSection(true);
                        }
                    });
                    const labelText = document.createElement('label');
                    labelText.className = "inputLabel";
                    labelText.textContent = setting.key + ": ";
                    label.appendChild(labelText);
                    label.appendChild(control);
                } else {
                    const control = document.createElement("input");
                    control.type = "checkbox";
                    control.checked = values[setting.key];
                    control.addEventListener("change", e => {
                        window.api.settings.setValue(section.key, setting.key, e.target.checked);
                    });
                    label.appendChild(control);
                    label.appendChild(document.createTextNode(" " + setting.key));
                }
                group.appendChild(label);
            }
        };
        createSection();
    }

    const closeContainer = document.createElement("div");
    closeContainer.className = "formDialogFooter";
    modalContents.appendChild(closeContainer);

    const close = document.createElement("button");
    close.className = "raised button-cancel block btnCancel formDialogFooterItem emby-button";
    close.textContent = "Close"
    close.addEventListener("click", () => {
        modalContainer.remove();
    });
    closeContainer.appendChild(close);
}
