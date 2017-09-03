#!/bin/bash

function mklink {
    if [ ! -L $2 ];then
        ln -s $1 $2
    fi
}

mkdir -p bin/Debug/Contents/Resources
mkdir -p bin/Release/Contents/Resources
mklink ../res res
mklink ../../res bin/res
mklink ../game.config game.config
mklink ../user.config user.config
mklink ../default.config default.config
mklink ../../game.config bin/game.config
mklink ../../user.config bin/user.config
mklink ../../default.config bin/default.config
mklink ../../../../../res bin/Debug/Contents/Resources/res
mklink ../../../../../game.config bin/Debug/Contents/Resources/game.config
mklink ../../../../../user.config bin/Debug/Contents/Resources/user.config
mklink ../../../../../default.config bin/Debug/Contents/Resources/default.config
mklink ../../../../../res bin/Release/Contents/Resources/res
mklink ../../../../../game.config bin/Release/Contents/Resources/game.config
mklink ../../../../../user.config bin/Release/Contents/Resources/user.config
mklink ../../../../../default.config bin/Release/Contents/Resources/default.config
mklink ../external/GamePlay-deps/lua-5.2.3/src lua
mklink ../external/GamePlay-deps/png-1.6.15/scripts/pnglibconf.h.prebuilt pnglibconf.h
mklink ../external/GamePlay-deps/zlib-1.2.8/zconf.h.included zconf.h
