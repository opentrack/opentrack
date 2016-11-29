#!/bin/sh

build_dir=build-msvc15

set -e

dir="$(dirname -- "$0")"
cd "$dir/.."

pushd "./$build_dir" >/dev/null
cmake --build . --target i18n >/dev/null
popd >/dev/null

rel="$(git describe --tag --alw)"

rm -f "$rel"
find . -wholename "?*/lang/stub.ts" | zip -q9 "$build_dir/$rel-i18n-stub.zip" -@
