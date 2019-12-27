#!/bin/sh

# exit when any command fails
set -e

# keep track of the last executed command
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# echo an error message before exiting
trap 'echo "--\n--\n--\n--\n\"${last_command}\" command failed with exit code $?."' EXIT 

APPNAME=opentrack
# Alternative we could look at https://github.com/arl/macdeployqtfix ??

#macosx directory
dir="$1" 
test -n "$dir" || exit 1
# install directory
install="$2"
test -n "$install" || exit 1
version="$3"
test -n "$version" || exit 1

tmp="$(mktemp -d "/tmp/$APPNAME-tmp.XXXXXXX")"
test $? -eq 0 || exit 1

# Add framework and other libraries
macdeployqt "$install/$APPNAME.app" -libpath="$install/$APPNAME.app/Contents/MacOS"

# Fixup dylib linker issues
sh "$dir/install-fail-tool" "$install/$APPNAME.app/Contents/Frameworks"

# Copy our own plist and set correct version
cp "$dir/Info.plist" "$install/$APPNAME.app/Contents/"
cp "$dir/PkgInfo" "$install/$APPNAME.app/Contents/"
cp "$dir/opentrack.sh" "$install/$APPNAME.app/Contents/MacOS/"
sed -i '' -e "s#@OPENTRACK-VERSION@#$version#g" "$install/$APPNAME.app/Contents/Info.plist" || exit 1

# Create an 512 resolution size (for retina displays mostly)
#gm convert -size 512x512 "$dir/../gui/images/opentrack.png" "$tmp/opentrack.png"
convert "$dir/../gui/images/opentrack.png" -filter triangle -resize 512x512 "$tmp/opentrack.png"

# Build iconset 
mkdir "$tmp/$APPNAME.iconset" || exit 1
sips -z 16 16     "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_16x16.png" || exit 1
sips -z 32 32     "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_16x16@2x.png" || exit 1
sips -z 32 32     "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_32x32.png" || exit 1
sips -z 64 64     "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_32x32@2x.png" || exit 1
sips -z 128 128   "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_128x128.png" || exit 1
sips -z 256 256   "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_128x128@2x.png" || exit 1
sips -z 512 512   "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_256x256@2x.png" || exit 1
sips -z 512 512   "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_512x512.png" || exit 1
iconutil -c icns -o "$install/$APPNAME.app/Contents/Resources/$APPNAME.icns" "$tmp/$APPNAME.iconset"
rm -rf "$tmp"

#Build DMG
#https://github.com/andreyvit/create-dmg
rm -rf $install/../$version.dmg
create-dmg \
  --volname "$APPNAME" \
  --volicon "$install/$APPNAME.app/Contents/Resources/$APPNAME.icns" \
  --window-pos 200 120 \
  --window-size 800 450 \
  --icon-size 100 \
  --background "$dir/dmgbackground.png" \
  --icon "$APPNAME.app" 200 190 \
  --hide-extension "$APPNAME.app" \
  --app-drop-link 600 185 \
  "$version.dmg" \
  "$install/$APPNAME.app"

# Check if we have a DMG otherwise fail
FILE=$install/../$version.dmg    
if [ -f $FILE ]; then
   ls -ial $install/../*.dmg    
   exit 0
else
   echo "Failed to create $FILE"
   exit 2
fi
