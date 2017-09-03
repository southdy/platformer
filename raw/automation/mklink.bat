@echo off
CALL mklink /H "game.config" "../game.config"
CALL mklink /H "user.config" "../user.config"
CALL mklink /H "default.config" "../default.config"
CALL mklink /J "res" "../res"
CALL mklink /J "lua" "../external/GamePlay-deps/lua-5.2.3/src"
CALL mklink /H "pnglibconf.h" "../external/GamePlay-deps/png-1.6.15/scripts/pnglibconf.h.prebuilt"
CALL mklink /H "zconf.h" "../external/GamePlay-deps/zlib-1.2.8/zconf.h.included"

call :make_links Debug
call :make_links Release

goto :exit

:make_links
set CONFIG_BIN_DIR=bin
mkdir "%CONFIG_BIN_DIR%"
CALL mklink /H "%CONFIG_BIN_DIR%/OpenAL.dll" "../external/GamePlay/bin/windows/OpenAL.dll"
CALL mklink /H "%CONFIG_BIN_DIR%/game.config" "../game.config"
CALL mklink /H "%CONFIG_BIN_DIR%/user.config" "../user.config"
CALL mklink /H "%CONFIG_BIN_DIR%/default.config" "../default.config"
CALL mklink /J "%CONFIG_BIN_DIR%/res" "../res"
goto:eof

:exit
