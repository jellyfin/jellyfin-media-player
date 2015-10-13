#!/bin/sh

set -e

rm -rf libass ffmpeg mpv

git clone --depth 1 git://github.com/libass/libass
git clone --depth 1 git://github.com/ffmpeg/ffmpeg
git clone git://github.com/mpv-player/mpv

cd libass
./autogen.sh
./configure
make -j 4
sudo make install

cd ..

cd ffmpeg
./configure --enable-gnutls --enable-shared --disable-static
make -j 4
sudo make install

cd ..

cd mpv
./bootstrap.py
./waf configure --enable-libmpv-shared --disable-cplayer
./waf build -j5
sudo ./waf install

cd ..
