#!/bin/bash

if [[ "$COMPILER" == "Emscripten" ]]; then
  docker pull thewtex/cross-compiler-browser-asmjs
  CMAKE_TOOLCHAIN_FILE=`docker inspect --format='{{range .Config.Env}}{{println .}}{{end}}' thewtex/cross-compiler-browser-asmjs | grep CMAKE_TOOLCHAIN_FILE | sed 's/^.*=//'`
  export CONFIG_ARGS="-DBoost_INCLUDE_DIR=/usr/local/include -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE"
  export RUN="docker run --rm \
                  -v $PWD:/mfast \
                  -v /usr/include/boost:/usr/local/include/boost \
                  -e AR= \
                  -e CXX= \
                  -w /mfast/build \
                  -t thewtex/cross-compiler-browser-asmjs "
else
  export CONFIG_ARGS="-DCMAKE_CXX_COMPILER=$COMPILER"
fi                  