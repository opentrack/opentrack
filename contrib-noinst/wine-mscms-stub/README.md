mscms stub for Wine and CrossOver
=================================

Qt 6.10 and newer link the Windows color system: their qwindows platform
plugin calls mscms.dll.WcsCreateIccProfile while reading the display
color profile. Wine implements the rest of mscms but that function is an
unimplemented stub, and wine aborts the whole process the moment it is
called:

    wine: Call from 0x... to unimplemented function mscms.dll.WcsCreateIccProfile, aborting

Every opentrack build since the Qt 6 upgrade dies this way under Wine,
CrossOver and Proton before a window can appear.

This directory contains a replacement mscms.dll that exports the four
functions qwindows.dll needs and returns NULL from all of them. Qt then
falls back to the default sRGB color space and starts normally.

Installing
----------

1. copy mscms.dll into the opentrack install directory, next to
   opentrack.exe (wine looks for native DLLs there first)
2. tell wine to prefer the native dll for this program only. Import this
   into the wine registry (regedit /S file.reg):

        REGEDIT4

        [HKEY_CURRENT_USER\Software\Wine\AppDefaults\opentrack.exe\DllOverrides]
        "mscms"="native"

   In CrossOver, "Wine configuration" in the bottle (or regedit through
   Run Command) edits the same keys. The AppDefaults key scopes the
   override to opentrack.exe alone, so other programs in the bottle keep
   using the builtin mscms.

Rebuilding
----------

any mingw or llvm-mingw toolchain:

    x86_64-w64-mingw32-gcc -shared -o mscms.dll mscms-stub.c

The prebuilt mscms.dll here was built with llvm-mingw (clang) for
x86_64. opentrack's Windows builds are x86_64 only.
