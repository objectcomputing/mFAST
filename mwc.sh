#!/bin/bash

#export MFAST_ROOT=$PWD
#export BOOST_ROOT=

if [[ "$OSTYPE" == "darwin"* ]]
then
    EXTRA_CONFIG="-value_template platforms=macosx"
fi

if [[ "$1" == "-debug" ]]
then
   EXTRA_CONFIG="$EXTRA_CONFIG -value_template configurations=Debug -value_template lib_modifier="
fi

$MPC_ROOT/mwc.pl -include MPC/config -type make $EXTRA_CONFIG mfast.mwc
