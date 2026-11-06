#!/bin/sh
# Make an installed opentrack.app bundle self-contained.
#
# The build system bakes absolute run paths and Homebrew library references
# into every Mach-O file. macdeployqt only fixes up Qt, so without this pass
# the bundle either loads duplicate library copies from the build machine
# (which aborts at startup with OpenMP's "duplicate libomp" error) or fails
# to resolve dependencies that were rewritten but never copied.
#
# Repeated until stable:
#   1. copy referenced Homebrew libraries into Contents/Frameworks
#   2. rewrite those references to @executable_path/../Frameworks/...
#   3. drop absolute run paths, keep @-relative ones
#   4. add @executable_path/../Frameworks where @rpath entries need it
# Then it verifies that no reference points outside the bundle.
#
# usage: fixup-bundle.sh /path/to/opentrack.app

set -e

app="$1"
if [ -z "$app" ] || [ ! -d "$app/Contents" ]; then
    echo "usage: $0 /path/to/opentrack.app" >&2
    exit 1
fi

fw="$app/Contents/Frameworks"
mkdir -p "$fw"

brew_prefix=/opt/homebrew
if command -v brew > /dev/null 2>&1; then
    brew_prefix=`brew --prefix`
elif [ ! -d "$brew_prefix" ]; then
    brew_prefix=/usr/local
fi

tmp=`mktemp -d /tmp/opentrack-fixup.XXXXXXX`
trap 'rm -rf "$tmp"' EXIT
changed="$tmp/changed"
errors="$tmp/errors"
: > "$errors"
rounds=0

is_macho() {
    file -b "$1" 2> /dev/null | grep -q '^Mach-O'
}

# otool -L prints a dylib's own install name as the first entry, if any
dylib_id() {
    otool -D "$1" 2> /dev/null | sed -n '2p'
}

load_deps() {
    otool -L "$1" | tail -n +2 | awk '{ print $1 }'
}

rpaths() {
    otool -l "$1" | awk '/LC_RPATH/ { f=1; next } f && $1 == "path" { print $2; f=0 }'
}

find_library() {
    rel="$1"
    for d in "$brew_prefix/lib" "$brew_prefix"/opt/*/lib "$brew_prefix"/Cellar/*/*/lib \
             /usr/local/lib /usr/local/opt/*/lib /usr/local/Cellar/*/*/lib
    do
        if [ -f "$d/$rel" ]; then
            echo "$d/$rel"
            return 0
        fi
    done
    find "$brew_prefix" /usr/local -name "`basename "$rel"`" -type f 2> /dev/null | head -n 1
}

copy_into() {
    # $1 source file, $2 destination file
    if [ -e "$2" ]; then
        return 0
    fi
    mkdir -p "`dirname "$2"`"
    cp "$1" "$2"
    chmod u+w "$2"
}

while [ "$rounds" -lt 12 ]; do
    rounds=$((rounds + 1))
    : > "$changed"

    find "$app/Contents" -type f | while IFS= read -r f; do
        is_macho "$f" || continue
        chmod u+w "$f" 2> /dev/null || true

        id=`dylib_id "$f"`
        need_rpath=0

        for dep in `load_deps "$f"`; do
            if [ "$dep" = "$id" ]; then
                continue
            fi
            case "$dep" in
            /usr/lib/* | /System/*)
                continue ;;
            @rpath/*)
                need_rpath=1
                continue ;;
            @executable_path/*)
                target="$app/Contents/MacOS/${dep#@executable_path/}" ;;
            @loader_path/*)
                target="`dirname "$f"`/${dep#@loader_path/}" ;;
            *)
                target="" ;;
            esac

            case "$dep" in
            @executable_path/* | @loader_path/*)
                if [ ! -e "$target" ]; then
                    src=`find_library "`basename "$target"`"`
                    if [ -n "$src" ]; then
                        copy_into "$src" "$target"
                        echo x >> "$changed"
                    else
                        echo "$f -> $dep (missing)" >> "$errors"
                    fi
                fi
                continue ;;
            esac

            # absolute reference, most likely Homebrew
            rest=""
            case "$dep" in
            */lib/*) rest="${dep#*/lib/}" ;;
            esac
            if [ -z "$rest" ]; then
                echo "$f -> $dep (unsupported path)" >> "$errors"
                continue
            fi
            target="$fw/$rest"
            if [ ! -e "$target" ]; then
                src=`find_library "$rest"`
                if [ -z "$src" ]; then
                    echo "$f -> $dep (not found)" >> "$errors"
                    continue
                fi
                case "$rest" in
                *.framework/*)
                    frag="${rest%%.framework/*}.framework"
                    src_root=`echo "$src" | sed "s|\(.*/$frag\).*|\1|"`
                    dst_root="$fw/$frag"
                    if [ ! -e "$dst_root" ]; then
                        cp -R "$src_root" "$dst_root"
                        chmod -R u+w "$dst_root"
                    fi
                    ;;
                *)
                    copy_into "$src" "$target"
                    ;;
                esac
                echo x >> "$changed"
            fi
            install_name_tool -change "$dep" "@executable_path/../Frameworks/$rest" "$f"
            echo x >> "$changed"
        done

        if [ "$need_rpath" = 1 ] && ! rpaths "$f" | grep -qxF '@executable_path/../Frameworks'; then
            install_name_tool -add_rpath '@executable_path/../Frameworks' "$f"
            echo x >> "$changed"
        fi

        for rp in `rpaths "$f"`; do
            case "$rp" in
            @*) continue ;;
            esac
            install_name_tool -delete_rpath "$rp" "$f"
            echo x >> "$changed"
        done
    done

    if [ ! -s "$changed" ]; then
        break
    fi
done

# Verify nothing outside the bundle is referenced any more.
# Errors from the fix phase above are kept: they are real failures too.
find "$app/Contents" -type f | while IFS= read -r f; do
    is_macho "$f" || continue
    id=`dylib_id "$f"`

    for dep in `load_deps "$f"`; do
        if [ "$dep" = "$id" ]; then
            continue
        fi
        case "$dep" in
        /usr/lib/* | /System/*)
            continue ;;
        @executable_path/*)
            target="$app/Contents/MacOS/${dep#@executable_path/}"
            if [ ! -e "$target" ]; then
                echo "$f -> $dep (missing)" >> "$errors"
            fi
            continue ;;
        @loader_path/*)
            target="`dirname "$f"`/${dep#@loader_path/}"
            if [ ! -e "$target" ]; then
                echo "$f -> $dep (missing)" >> "$errors"
            fi
            continue ;;
        @rpath/*)
            rel="${dep#@rpath/}"
            resolved=0
            for rp in `rpaths "$f"`; do
                case "$rp" in
                @executable_path/*) base="$app/Contents/MacOS/${rp#@executable_path/}" ;;
                @loader_path/*) base="`dirname "$f"`/${rp#@loader_path/}" ;;
                *) continue ;;
                esac
                if [ -e "$base/$rel" ]; then
                    resolved=1
                    break
                fi
            done
            if [ "$resolved" = 0 ]; then
                echo "$f -> $dep (unresolved)" >> "$errors"
            fi
            continue ;;
        *)
            echo "$f -> $dep (absolute reference remains)" >> "$errors"
            continue ;;
        esac
    done

    for rp in `rpaths "$f"`; do
        case "$rp" in
        @*) continue ;;
        *)
            echo "$f: absolute run path $rp remains" >> "$errors" ;;
        esac
    done
done

if [ -s "$errors" ]; then
    echo "fixup-bundle: unresolved references:" >&2
    sort -u "$errors" >&2
    exit 1
fi

echo "fixup-bundle: bundle is self-contained ($rounds round(s))"
