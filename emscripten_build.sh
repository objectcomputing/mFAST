#!/bin/bash

set -e

NATIVE_BUILD_ROOT=${NATIVE_BUILD_ROOT:-$PWD/_build}

if [ -z ${BOOST_ROOT+x} ]; then
    BOOST_ROOT=`grep Boost_INCLUDE_DIR:PATH $NATIVE_BUILD_ROOT/CMakeCache.txt | cut -d '=' -f 2`
fi

MFAST_ROOT=`grep CMAKE_HOME_DIRECTORY $NATIVE_BUILD_ROOT/CMakeCache.txt | cut -d '=' -f 2`

RUN="docker run --rm \
                -v $MFAST_ROOT:/mfast \
                -v $PWD/_emscripten_build:/em_build/mfast \
                -v $NATIVE_BUILD_ROOT:/native_build/mfast \
                -v $BOOST_ROOT/boost:/usr/src/boost/boost \
                -w /em_build/mfast
                -t thewtex/cross-compiler-browser-asmjs "


mkdir -p _emscripten_build

cat << 'EOF' > _emscripten_build/fast_type_gen
#/bin/bash

for file in "$@"
do
    filebase=${file%.*}
    cp /native_build${filebase}.h .
    cp /native_build${filebase}.inl .
    cp /native_build${filebase}.cpp .
done
EOF
chmod +x _emscripten_build/fast_type_gen

$RUN emcmake cmake -DCMAKE_BUILD_TYPE=RELEASE -DBoost_INCLUDE_DIR=/usr/src/boost -DFAST_TYPE_GEN=/em_build/mfast/fast_type_gen /mfast
$RUN make
$RUN make test