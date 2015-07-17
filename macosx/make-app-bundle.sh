#!/bin/sh

APPNAME=opentrack

dir="$1"
test -n "$dir" || exit 1
install="$1"
test -n "$install" || exit 1
output_dir="$3"
test -n "$output_dir" || exit 1
version="$4"
test -n "$version" || exit 1

tmp="$(mktemp -d "/tmp/$APPNAME-tmp.XXXXXXX")"
test $? -eq 0 || exit 1

cp -R "$dir/opentrack.app" "$tmp" || exit 1
cp -R "$install/"* "$tmp/$APPNAME.app/Contents/MacOS" || exit 1
sed -i -e "s#@OPENTRACK-VERSION@#$version#g" "$tmp/$APPNAME.app/Contents/Info.plist" || exit 1

rm -rf "$tmp/$APPNAME.iconset"
mkdir "$tmp/$APPNAME.iconset"

sips -z 16 16     "$dir/../facetracknoir/images/facetracknoir.png" --out "$tmp/$APPNAME.iconset/icon_16x16.png" || exit 1
sips -z 32 32     "$dir/../facetracknoir/images/facetracknoir.png" --out "$tmp/$APPNAME.iconset/icon_16x16@2x.png" || exit 1
sips -z 32 32     "$dir/../facetracknoir/images/facetracknoir.png" --out "$tmp/$APPNAME.iconset/icon_32x32.png" || exit 1
sips -z 64 64     "$dir/../facetracknoir/images/facetracknoir.png" --out "$tmp/$APPNAME.iconset/icon_32x32@2x.png" || exit 1
sips -z 128 128   "$dir/../facetracknoir/images/facetracknoir.png" --out "$tmp/$APPNAME.iconset/icon_128x128.png" || exit 1

iconutil -c icns -o "$tmp/$APPNAME.app/Contents/Resources/$APPNAME.icns" "$tmp/$APPNAME.iconset"
rm -r "$tmp/$APPNAME.iconset"

cd "$tmp" || exit 1
zip -9 "$output_dir/$APPNAME-$version.zip" "$APPNAME.app" || exit 1
ls -lh "$output_dir/$APPNAME-$version.zip"