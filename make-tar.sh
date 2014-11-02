#!/bin/sh

prefix="$1"
filename="$2"

if  : &&
    make install &&
    cd $(dirname -- "${prefix}") &&
    zip -9r "${filename}" $(basename "${prefix}")
then
    ls -lh -- "${filename}"
else
    rm -fv -- "${filename}"
    exit 1
fi

exit 0
