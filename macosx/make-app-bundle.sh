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
test -n "$dir" 
# install directory
install="$2"
test -n "$install" 
version="$3"
test -n "$version" 

tmp="$(mktemp -d "/tmp/$APPNAME-tmp.XXXXXXX")"
test $? -eq 0 


# Add rpath for application so it can find the libraries
#install_name_tool -add_rpath @executable_path/../Frameworks "$install/$APPNAME.app/Contents/MacOS/$APPNAME"

# Copy our own plist and set correct version
cp "$dir/Info.plist" "$install/$APPNAME.app/Contents/"
sed -i '' -e "s#@OPENTRACK-VERSION@#$version#g" "$install/$APPNAME.app/Contents/Info.plist" 

# Copy PkgInfo
cp "$dir/PkgInfo" "$install/$APPNAME.app/Contents/"

# Copy plugins
mkdir -p "$install/$APPNAME.app/Contents/MacOS/Plugins"
cp -r "$install/Plugins" "$install/$APPNAME.app/Contents/MacOS/"

# Use either of these, two of them at the same time will break things!
macdeployqt "$install/$APPNAME.app" -libpath="$install/Library"
#sh "$dir/install-fail-tool" "$install/$APPNAME.app/Contents/Frameworks"

# Create an 512 resolution size for the icon (for retina displays mostly)
#gm convert -size 512x512 "$dir/../gui/images/opentrack.png" "$tmp/opentrack.png"
convert "$dir/../gui/images/opentrack.png" -filter triangle -resize 512x512 "$tmp/opentrack.png"

# Build iconset 
mkdir "$tmp/$APPNAME.iconset" 
sips -z 16 16     "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_16x16.png" 
sips -z 32 32     "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_16x16@2x.png" 
sips -z 32 32     "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_32x32.png" 
sips -z 64 64     "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_32x32@2x.png" 
sips -z 128 128   "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_128x128.png" 
sips -z 256 256   "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_128x128@2x.png" 
sips -z 512 512   "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_256x256@2x.png" 
sips -z 512 512   "$tmp/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_512x512.png" 
iconutil -c icns -o "$install/$APPNAME.app/Contents/Resources/$APPNAME.icns" "$tmp/$APPNAME.iconset"
rm -rf "$tmp"

#Build DMG
#https://github.com/andreyvit/create-dmg
rm -rf $install/../*.dmg
create-dmg \
  --volname "$APPNAME" \
  --volicon "$install/$APPNAME.app/Contents/Resources/$APPNAME.icns" \
  --window-pos 200 120 \
  --window-size 800 450 \
  --icon-size 80 \
  --background "$dir/dmgbackground.png" \
  --icon "$APPNAME.app" 200 180 \
  --app-drop-link 420 180 \
  --hide-extension "$APPNAME.app" \
  --no-internet-enable \
  --add-folder "Document" "$install/doc" 20 40 \
  --add-folder "Xplane-Plugin" "$install/xplane" 420 40 \
  --add-folder "thirdparty" "$install/thirdparty" 620 40 \
  "$version.dmg" \
  "$install/$APPNAME.app"

# Check if we have a DMG otherwise fail
FILE=$install/../$version.dmg
if [ -f $FILE ]; then
   ls -ial $install/../*.dmg
else
   echo "Failed to create ${FILE}"
   exit 2
fi
