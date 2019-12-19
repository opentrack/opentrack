#!/bin/sh

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

# Build iconset 
mkdir "$tmp/$APPNAME.iconset" || exit 1
sips -z 16 16     "$dir/../gui/images/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_16x16.png" || exit 1
sips -z 32 32     "$dir/../gui/images/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_16x16@2x.png" || exit 1
sips -z 32 32     "$dir/../gui/images/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_32x32.png" || exit 1
sips -z 64 64     "$dir/../gui/images/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_32x32@2x.png" || exit 1
sips -z 128 128   "$dir/../gui/images/opentrack.png" --out "$tmp/$APPNAME.iconset/icon_128x128.png" || exit 1
iconutil -c icns -o "$install/$APPNAME.app/Contents/Resources/$APPNAME.icns" "$tmp/$APPNAME.iconset"
rm -rf "$tmp"

# Zip it up
#zip -9r "$install/$version-macosx.zip" "$APPNAME.app" || exit 1
#ls -lh "$install/$version-macosx.zip"
