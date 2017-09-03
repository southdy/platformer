set ROOTDIR=%CD%
set LIBDIR=%1

echo "Amalgamating target dir: %LIBDIR%"

cd %LIBDIR%
mkdir tmp
del gameplay-deps.lib
xcopy /D *.lib tmp

LIB.EXE /OUT:gameplay-deps.lib tmp\*

cd %ROOTDIR%
