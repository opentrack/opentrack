#!/bin/sh

cd -- "$(dirname -- "$0")" &&
exec ./opentrack -platformpluginpath "$(pwd)" "$@"

exit 1
