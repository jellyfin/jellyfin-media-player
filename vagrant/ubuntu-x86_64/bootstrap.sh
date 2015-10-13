#!/bin/bash

set -e

if [ ! -e /swapfile ]; then
  fallocate -l 16G /swapfile
  chmod 600 /swapfile
  mkswap /swapfile
  swapon /swapfile
  echo "/swapfile none swap sw 0 0" >>/etc/fstab
fi

cp /vagrant/sources.list /etc/apt/sources.list

add-apt-repository -y ppa:beineri/opt-qt55-trusty
apt-get update -y
apt-get upgrade -y
apt-get install -y git cmake ninja-build build-essential libssl-dev libgles2-mesa-dev qt54-meta-full pkg-config autoconf libtool libfreetype6-dev libfribidi-dev libfontconfig-dev libharfbuzz-dev yasm libgnutls-dev libbz2-dev libxrandr-dev libglew-dev libsdl2-dev libcec-dev

sudo -u vagrant cp /vagrant/build-deps.sh .
chmod 755 build-deps.sh
sudo -u vagrant ./build-deps.sh