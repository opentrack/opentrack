#!/bin/sh

dir="$(dirname -- "$0")"
exec "$dir"/opentrack.bin -platformpluginpath "$dir" "$@"
