#!/bin/sh

cleanup() {
    for i in cov-build ninja; do
        taskkill -f -t -im "$i.exe" 2>/dev/null 1>&2
    done
    rm -f "$myfile"
}

signal() {
    set +e
    trap '' EXIT
    echo "error: $1" 1>&2
    cleanup
    exit 1
}

export PATH="/d/dev/cov-analysis-win64-8.7.0/bin:/c/msys64/mingw32/bin:/c/msys64/usr/bin:$PATH"

for k in HUP INT QUIT ILL BUS FPE SEGV PIPE; do
    trap "signal 'got fatal signal SIG'$k" SIG"$k"
done

trap 'signal "fatal return $?"' EXIT

set -e

mydate="$(date --iso-8601=minutes)"
mydir="$(dirname -- "$0")"
myfile="opentrack-"$mydate".7z"

cd "$mydir"
for k in opencv aruco libovr-025  libovr-042 libovr-080; do
    ninja -C "./$k"
done

cd "./opentrack"

cmake .
ninja32 clean
cov-build --dir cov-int ninja -v -j1
rm -f "../$myfile" || true
7za -mx a "../$myfile" cov-int
trap '' EXIT
exit 0
