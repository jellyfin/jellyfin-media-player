#!/bin/bash
cd "$(dirname "$0")"

function download_compat {
    if [[ "$AZ_CACHE" != "" ]]
    then
        download_id=$(echo "$2" | md5sum | sed 's/ .*//g')
        if [[ -e "$AZ_CACHE/$3/$download_id" ]]
        then
            echo "Cache hit: $AZ_CACHE/$3/$download_id"
            cp "$AZ_CACHE/$3/$download_id" "$1"
            return
        elif [[ "$3" != "" ]]
        then
            rm -r "$AZ_CACHE/$3" 2> /dev/null
        fi
    fi
    if [[ "$(which wget 2>/dev/null)" != "" ]]
    then
        wget -qO "$1" "$2"
    else [[ "$(which curl)" != "" ]]
        curl -sL "$2" > "$1"
    fi
    if [[ "$AZ_CACHE" != "" ]]
    then
        echo "Saving to: $AZ_CACHE/$3/$download_id"
        mkdir -p "$AZ_CACHE/$3/"
        cp "$1" "$AZ_CACHE/$3/$download_id"
    fi
}

function get_resource_version {
    curl -s --head https://github.com/"$1"/releases/latest | \
        grep -i '^location: ' | sed 's/.*tag\///g' | tr -d '\r'
}

if [[ "$1" == "--gen-fingerprint" ]]
then
    (
        get_resource_version iwalton3/jellyfin-web-jmp
    ) | tee az-cache-fingerprint.list
    exit 0
fi

# Download web client
update_web_client="no"
mkdir -p build
if [[ ! -e "build/dist" ]]
then
    update_web_client="yes"
elif [[ -e ".last_wc_version" ]]
then
    if [[ "$(get_resource_version iwalton3/jellyfin-web-jmp)" != "$(cat .last_wc_version)" ]]
    then
        update_web_client="yes"
    fi
fi

if [[ "$update_web_client" == "yes" ]]
then
    echo "Downloading web client..."
    wc_version=$(get_resource_version iwalton3/jellyfin-web-jmp)
    download_compat dist.zip "https://github.com/iwalton3/jellyfin-web-jmp/releases/download/$wc_version/dist.zip" "wc"
    rm -r build/dist 2> /dev/null
    rm -r dist 2> /dev/null
    unzip dist.zip > /dev/null && rm dist.zip
    mv dist build/
    echo "$wc_version" > .last_wc_version
fi

