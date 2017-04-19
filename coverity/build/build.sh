#!/bin/sh

function cleanup() {
    for i in cmake cov-build ninja cc1 cc1plus collect2 lto1 lto-wrapper gcc g++; do
        taskkill -f -im "$i.exe" >/dev/null 2>&1 && echo "$i killed" 1>&2
    done
    rm -f "$myfile"
}

function signal() {
    set +e
    trap '' EXIT
    echo "error: $1" 1>&2
    cleanup
}

export PATH="/d/dev/cov-analysis-win64-8.7.0/bin:/mingw32/bin:/bin:$PATH"

for k in HUP INT QUIT ILL BUS FPE SEGV PIPE; do
    trap "signal 'got fatal signal SIG'$k; exit 1" SIG"$k"
done

trap 'signal "fatal return $?"' EXIT

set -e

mydate="$(date --iso-8601=minutes)"
mydir="$(dirname -- "$0")"
myfile="$(pwd)/opentrack-"$mydate".tar.xz"

cd "$mydir"
for k in opencv aruco libovr-025  libovr-042  libovr-080; do
    ninja -C "./$k"
done

cd "./opentrack"

cmake .
ninja clean
cov-build --dir cov-int ninja
bsdtar Jcf ../opentrack-"$mydate".tar.xz cov-int
trap '' EXIT
exit 0
