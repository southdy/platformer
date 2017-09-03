#!/bin/bash

if [ $# -ne 2 ];
then
    echo "Usage: gnu-amalgamate.sh <ar command> <target directory>"
    exit 1
fi

CWD=`pwd`
AR="$1"
TARGET="$2"

if [ ! -e $AR ];
then
    echo "No such file: $AR"
    exit 1
fi

if [ ! -d $TARGET ];
then
    echo "No such target directory: $AR"
    exit 1
fi

SIZE="$(stat -c%s $TARGET/libgameplay-deps.a)"
PREVIOUS_TIME="$(stat -c%Y $TARGET/libgameplay-deps.a)"
RELINK=0
if [ $SIZE -ge 100000 ]
then
for file in $TARGET/* ; do
    TIME="$(stat -c%Y $file)"
    if [ $TIME -gt $PREVIOUS_TIME ]|| [ $TIME -eq 0 ];
    then
        RELINK=1
    fi
done
else
    RELINK=1
fi

if [ $RELINK -eq 1 ];
then
    echo "Using ar : $AR"
    echo "Amalgamating target static libs $TARGET"

    cd $TARGET
    mkdir tmp
    rm -f libgameplay-deps.a
    cp *.a ./tmp

    # Build an MRI script file
    MRI="$CWD/deps.mri"
    echo "create libgameplay-deps.a" >$MRI
    for file in ./tmp/* ; do
        if [ -e "$file" ];
        then
            echo "addlib $file" >>$MRI
        fi
    done
    echo "save" >>$MRI
    echo "end" >>$MRI

    echo "Executing..."
    cat $MRI

    # Now actually do something
    $AR -M < $MRI

    # Clean up
    rm $MRI
    rm -rf ./tmp

    cd $CWD
fi
