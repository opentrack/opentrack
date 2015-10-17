#!/bin/sh

prefix="$1"
filename="$2"

rm -fv "$filename"

if  : &&
    cd $(dirname -- "${prefix}") &&
    zip -9r "${filename}" $(basename "${prefix}")
then
    ls -lh -- "${filename}"
    case "$(uname -s)" in
        CYGWIN_*) ls -lh -- "$(cygpath -w -- "$filename")";;
    esac
else
    rm -fv -- "${filename}"
    exit 1
fi

exit 0
