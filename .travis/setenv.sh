#!/bin/bash

if [[ "$COMPILER" == "Emscripten" ]]; then
  CMAKE_TOOLCHAIN_FILE=`docker inspect --format='{{range .Config.Env}}{{println .}}{{end}}' thewtex/cross-compiler-browser-asmjs | grep CMAKE_TOOLCHAIN_FILE | sed 's/^.*=//'`
  CONFIG_ARGS="-DBoost_INCLUDE_DIR=/usr/local/include -DCMAKE_TOOLCHAIN_FILE=$CMAKE_TOOLCHAIN_FILE"
  RUN="docker run --rm \
                  -v $PWD:/mfast \
                  -v /usr/include/boost:/usr/local/include/boost \
                  -e "AR="
                  -e "CXX="
                  -w "/mfast"
                  -t thewtex/cross-compiler-browser-asmjs "
else
  CONFIG_ARGS="-DCMAKE_CXX_COMPILER=$COMPILER"
fi                  