#!/bin/bash

sudo apt-get install python2.7 nodejs default-jre

# install cmake 2.8.8
# * github.com/travis-ci-tester/travis-test-cmake
wget https://launchpad.net/ubuntu/+source/cmake/2.8.8-2ubuntu1/+build/3441442/+files/cmake_2.8.8-2ubuntu1_amd64.deb
wget https://launchpad.net/ubuntu/+archive/primary/+files/cmake-data_2.8.8-2ubuntu1_all.deb
sudo apt-get remove cmake-data cmake
sudo dpkg --install cmake-data_2.8.8-2ubuntu1_all.deb cmake_2.8.8-2ubuntu1_amd64.deb
# -- end

# install emscripten
wget https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz
tar zxvf emsdk-portable.tar.gz
cd emsdk_portable
./emsdk update
./emsdk install latest
./emsdk activate latest
