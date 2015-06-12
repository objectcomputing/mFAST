#!/bin/bash

sudo apt-get install python2.7 nodejs default-jre

# install cmake 2.8.8
# * github.com/travis-ci-tester/travis-test-cmake
wget http://www.cmake.org/files/v3.2/cmake-3.2.3-Linux-x86_64.tar.gz
mkdir cmake-dist
tar -xzf cmake-3.2.3-Linux-x86_64.tar.gz -C cmake-dist --strip-components=1
export PATH=$PWD/cmake-dist/bin:$PATH
# -- end

# install emscripten
wget https://s3.amazonaws.com/mozilla-games/emscripten/releases/emsdk-portable.tar.gz
tar zxvf emsdk-portable.tar.gz
cd emsdk_portable
./emsdk update
./emsdk install latest
./emsdk activate latest
