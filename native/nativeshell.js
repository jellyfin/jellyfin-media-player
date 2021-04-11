const viewdata = JSON.parse(window.atob("@@data@@"));
console.log(viewdata);

const features = [
    "filedownload",
    "displaylanguage",
    "htmlaudioautoplay",
    "htmlvideoautoplay",
    "externallinks",
    //"clientsettings",
    "multiserver",
    "remotecontrol",
];

const plugins = [
    'mpvVideoPlayer',
    'mpvAudioPlayer',
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

    //openClientSettings() {
    //    window.NativeInterface.openClientSettings();
    //},

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
    async init() {
        window.api = await createApi();
    },
    getDefaultLayout() {
        return "desktop";
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