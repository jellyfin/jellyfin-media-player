#!/bin/bash
./download_webclient.sh
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr/local/ ..
make -j4
sudo make 
sudo make install