#!/bin/sh

prefix="$1"
filename="$2"
bin="$3"

cmake --build "$bin" --target install || exit 1

if  : &&
    cd $(dirname -- "${prefix}") &&
    zip -9r "${filename}" $(basename "${prefix}")
then
    case "$USER,$(uname -s)" in
        # for the script see https://github.com/andreafabrizi/Dropbox-Uploader
        sthalik,CYGWIN_*)
            dropbox_uploader.sh -p upload "$filename" / &&
            l="$(dropbox_uploader.sh share "/$filename")" &&
            f="$(echo "$l" | tr -d '\n\r' | egrep -o 'https://[^ ]+$')" &&
            test -n "$f" && { echo "$f"; echo -n "$f" | putclip; }
            echo -ne '\a' ;;
        *) ls -lh -- "${filename}" ;;
    esac
else
    rm -fv -- "${filename}"
    exit 1
fi

exit 0
