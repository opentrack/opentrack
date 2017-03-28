#!/bin/sh

function cleanup() {
    killall cc1 cc1plus collect2 lto1 lto-wrapper gcc g++ i686-w64-mingw32-gcc i686-w64-mingw32-{c++,g++} 2>/dev/null
    rm -f "$myfile"
}

function signal() {
    set +e
    trap '' EXIT
    echo "error: $1" 1>&2
    cleanup
    exit 1
}

for k in HUP INT QUIT ILL BUS FPE SEGV PIPE; do
    trap "signal 'got fatal signal SIG'$k" SIG"$k"
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

ninja -C ./libovr-140 install

cd "./opentrack"

cmake .
ninja clean
cov-build --dir cov-int ninja
tar Jcf ../opentrack-"$mydate".tar.xz cov-int
trap '' EXIT
exit 0
