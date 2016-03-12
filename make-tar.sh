#!/bin/sh

prefix="$1"
filename="$2"

branch="$(git rev-parse --abbrev-ref HEAD)"

if : &&
    cd $(dirname -- "${prefix}") &&
    zip -9r "${filename}" $(basename "${prefix}")
then
    case "$branch" in
    unstable)
        case "$USER,$(uname -s)" in
            # for the script see https://github.com/andreafabrizi/Dropbox-Uploader
            sthalik,CYGWIN_*)
                dropbox_uploader.sh -p upload "$filename" /
                bn="$(basename -- "$filename")"
                l="$(dropbox_uploader.sh -q share /"$bn")"
                test -n "$l" && echo -n "$l" | putclip
                echo $l
                echo -ne '\a' ;;
            *) ls -lh -- "${filename}" ;;
        esac ;;
    *) ;;
    esac
else
    rm -fv -- "${filename}"
    exit 1
fi

exit 0
