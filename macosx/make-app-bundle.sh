#!/bin/sh

# exit when any command fails
set -e

# keep track of the last executed command
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG
# echo an error message before exiting
trap 'echo "--\n--\n--\n--\n\"${last_command}\" command exited with exit code $?."' EXIT 

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
code_sign_identity="${4:-"-"}"
echo "Code-Sign-Identity is $code_sign_identity"
osx_arch="${5:-'unknownarch'}"
tmp="$(mktemp -d "/tmp/$APPNAME-tmp.XXXXXXX")"
test $? -eq 0 


# Add rpath for application so it can find the libraries
#install_name_tool -add_rpath @executable_path/../Frameworks "$install/$APPNAME.app/Contents/MacOS/$APPNAME"


# Create an 512 resolution size for the icon (for retina displays mostly)
#gm convert -size 512x512 "$dir/../gui/images/opentrack.png" "$tmp/opentrack.png"
convert "$dir/../gui/images/opentrack.png" -filter triangle -resize 512x512 "$tmp/opentrack.png"

mkdir -p "$install/$APPNAME.app/Contents/Resources/"

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


# Copy our own plist and set correct version

cp "$dir/Info.plist" "$install/$APPNAME.app/Contents/"
sed -i '' -e "s#@OPENTRACK-VERSION@#$version#g" "$install/$APPNAME.app/Contents/Info.plist" 

# Copy PkgInfo
cp "$dir/PkgInfo" "$install/$APPNAME.app/Contents/"

# Copy plugins
mkdir -p "$install/$APPNAME.app/Contents/PlugIns"
mkdir -p "$install/$APPNAME.app/Contents/MacOS/"


cp -r "$install/Plugins/" "$install/$APPNAME.app/Contents/PlugIns/opentrack"
# Copy thirdparty dlls amd libs for usage of WINE
cp -r "$install/thirdparty/" "$install/$APPNAME.app/Contents/PlugIns/opentrack"

# Use either of these, two of them at the same time will break things!
macdeployqt "$install/$APPNAME.app" -libpath="$install/Library"
#sh "$dir/install-fail-tool" "$install/$APPNAME.app/Contents/Frameworks"


if [ ! -d "$install/xplane" ]
then
 mkdir -p "$install/xplane"
fi

if [ "$code_sign_identity" = "-" ]
then
  echo "sign to run locally"
  # Sign it to run it locally otherwise you'll get errors. Also I've noticed that
  # this might make the binaries also usable for other users, but macOS will complain very much and it might not even work. I gave up and bought an Apple Developer Membership
  codesign --force --deep --sign - "$install/$APPNAME.app"
else
  # You have a proper developer certificate:
  echo "sign as $code_sign_identity"
  codesign --force --deep --verify -vv --options runtime --timestamp --entitlements $dir/entitlements.plist --sign "$code_sign_identity" $install/$APPNAME.app
  find "$install/xplane/" "$install/doc/" "$install/thirdparty/" -type f -exec codesign --force --verify -vv --options runtime --sign "$code_sign_identity" {} \;
fi

#Build DMG
#https://github.com/andreyvit/create-dmg

FILE="${version}_${osx_arch}.dmg"

rm -f *.dmg
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
  "$FILE" \
  "$install/$APPNAME.app"

# Check if we have a DMG otherwise fail

if [ -f "$FILE" ]; then
  
  # sign the dmg if You have a proper developer certificate:
  if [ "$code_sign_identity" != "-" ]
  then
    codesign -vv --force -s "$code_sign_identity" "$FILE"
  fi
  
  #To notarize do this:
  #xcrun notarytool submit "$FILE" --apple-id appleid@example.com--team-id "TEAM_ID" --password "specific-app-password" --verbose --wait 
  #
  #xcrun stapler staple -v "$FILE"

  ls -ial "$FILE"
else
   echo "Failed to create $FILE"
   exit 2
fi


