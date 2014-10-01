#!/bin/sh

prefix="$1"
filename="$2"

if  : &&
    make install &&
    cd $(dirname -- "${prefix}") &&
    tar Jcf "${filename}" $(basename "${prefix}")
then
    ls -lh -- "${filename}"
else
    rm -fv -- "${filename}"
    exit 1
fi

exit 0
