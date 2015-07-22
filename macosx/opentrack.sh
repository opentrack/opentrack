#!/bin/sh

cd -- "$(dirname -- "$0")" &&
exec ./opentrack.bin -platformpluginpath "$(pwd)" "$@"

exit 1
