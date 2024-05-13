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

function get_webclient_version {
    curl https://repo.jellyfin.org/files/server/portable/latest-stable/any/ |
    tr '<>/' '\t' | grep '[0-9]\+\.[0-9]\+\.[0-9]\+' | cut -f 3 | cut -d_ -f2 |
    sed 's/\.[a-z][a-z]*//g' | sort -V | tail -n 1
}

if [[ "$1" == "--gen-fingerprint" ]]
then
    (
        get_webclient_version
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
    if [[ "$(get_webclient_version)" != "$(cat .last_wc_version)" ]]
    then
        update_web_client="yes"
    fi
fi

if [[ "$update_web_client" == "yes" ]]
then
    echo "Downloading web client..."
    wc_version=$(get_webclient_version)
    download_compat dist.tar.gz "https://repo.jellyfin.org/files/server/portable/latest-stable/any/jellyfin_${wc_version}.tar.gz" "wc"
    if [[ "$DOWNLOAD_ONLY" != "1" ]]
    then
        rm -r build/dist 2> /dev/null
        rm -r dist 2> /dev/null
        tar -xvf dist.tar.gz > /dev/null && rm dist.tar.gz
        mv "jellyfin/jellyfin-web" build/dist
        rm -r jellyfin
    fi
    echo "$wc_version" > .last_wc_version
fi

