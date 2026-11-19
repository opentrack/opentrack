Head tracking for Windows games on a Mac, with opentrack
=========================================================

This guide covers the full setup: tracking your head with a webcam on macOS and feeding it to a Windows game that runs in Wine, CrossOver or Proton on the same machine. Everything runs locally, no TrackIR hardware needed.

The short version of the architecture:

    macOS                                   Wine / CrossOver / Proton
    ------------------------------------    --------------------------------
    opentrack.app                           opentrack.exe (bridge)
      tracker: neuralnet (webcam)     -->     input:  UDP over network :4242
      filter: whatever you like               output: FreeTrack 2.0 Enhanced
                                              (TrackIR interface)
                                                    |
                                                    v  FreeTrack shared memory
                                              game -> FreeTrackClient.dll
                                                    -> "use TrackIR" in game settings

Two opentrack instances are needed because Wine does not expose Mac webcams to Windows programs. DirectShow and Media Foundation both enumerate zero video capture devices under CrossOver 26.3 (audio capture is mapped, video capture is not). This is a Wine platform gap, confirmed by the CodeWeavers forums and by testing: stock ffmpeg.exe -list_devices true -f dshow -i dummy lists microphones but no cameras. A commercial head tracker (Beam Eye Tracker) solves it the same way: native tracker on macOS, game connection into the Wine environment.

Part 1: why the official Windows build does not run under Wine
--------------------------------------------------------------

opentrack 2026.1.0 and later are built against Qt 6.10, and Qt 6.10's official Windows binaries link the Windows system ICU (icuuc.dll). Real Windows ships it, Wine does not, so the process dies before main() with:

    err:module:import_dll Library icuuc.dll (which is needed by L"C:\\...\\Qt6Core.dll") not found

The same Qt also calls mscms.dll.WcsCreateIccProfile during startup, which Wine stubs out and aborts on.

Both problems go away if you build opentrack yourself with the MinGW flavor of Qt, whose Qt6Core.dll has no ICU dependency at all. That is what part 2 does. For the mscms problem there is also a drop-in workaround in contrib-noinst/wine-mscms-stub, usable with any opentrack build including official ones.

Part 2: building the Windows opentrack on macOS
-----------------------------------------------

Cross-compiling works well and needs no Windows machine. Three downloads and about five minutes of building.

1. a clang-based mingw toolchain. llvm-mingw is the easiest: download macos-universal from https://github.com/mstorsjo/llvm-mingw/releases and untar it. The plain x86_64-w64-mingw32 triple in llvm-mingw is what we want. (Homebrew's mingw-w64 also works for compiling, but it is UCRT-flavored while Qt's MinGW binaries are msvcrt-flavored, which trips over Qt's entry point library; the WinMain fix in the source tree handles that too, but llvm-mingw avoids the topic entirely.)

2. Qt 6 for MinGW from the Qt archive. The layout under https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt6_6120/qt6_6120_mingw/ (adjust the version) contains one 7z per module, with the version baked into the file name, for example:

    6.12.0-0-202609280346qtbase-Windows-Windows_11_24H2-Mingw-Windows-Windows_11_24H2-X86_64.7z

   Download qtbase, qtsvg, qttools, qttranslations, the MinGW-w64 runtime archive, and qtserialport (from the addons directory), then extract them all into one directory. aqtinstall's list-qt --arch is currently broken for these, so direct URLs are the reliable path. Mirrors of qtproject work too; ftp.fau.de can lag behind, mirrors.dotsrc.org was current. The extracted tree is the Qt root: bin/, lib/, plugins/, plus libgcc_s_seh-1.dll, libstdc++-6.dll and libwinpthread-1.dll from the runtime archive.

3. opentrack itself, with submodules:

    git clone https://github.com/opentrack/opentrack
    cd opentrack && git submodule update --init --recursive

Configure and build:

    cmake -GNinja -S . -B build-win \
        -DCMAKE_TOOLCHAIN_FILE=$PWD/cmake/mingw-w64-x86_64-host.cmake \
        -DCMAKE_BUILD_TYPE=Release \
        -DQt6_DIR=/path/to/qt-win/lib/cmake/Qt6 \
        -DQT_HOST_PATH=$(brew --prefix qt) \
        -DCMAKE_MAKE_PROGRAM=$(which ninja)

    cmake --build build-win --target install

QT_HOST_PATH points at a native Qt of the same major version and supplies moc, uic, rcc and the translation tools, which are host programs and cannot run from the Windows binaries.

You need ninja (brew install ninja) and pkg-config is deliberately not consulted: video-ps3eye skips itself when cross-compiling, so no host libusb can leak in. Without OpenCV present, the OpenCV-based trackers (including neuralnet) and video-opencv are skipped, which is exactly what the bridge needs: it does not capture video, it only converts UDP to FreeTrack.

The result is build-win/install with opentrack.exe, Qt6Core/Gui/Network/Widgets.dll, platforms/qwindows.dll, the opentrack-*.dll modules including opentrack-proto-freetrack, and freetrackclient64.dll in modules/. Copy the whole directory into the bottle, for example to C:\Program Files (x86)\opentrack-headless. Wine's system msvcrt covers the CRT; the three mingw runtime dlls already sit next to the exe.

Part 3: the mscms workaround for the GUI
----------------------------------------

Without this, every opentrack build since the Qt 6 upgrade dies under Wine while creating its first window. The stub and instructions live in contrib-noinst/wine-mscms-stub: copy mscms.dll next to opentrack.exe and add the per-program DLL override documented there (one registry key, scoped to opentrack.exe so the rest of the bottle keeps the builtin mscms). After that the GUI runs normally in CrossOver.

Part 4: bridge configuration
----------------------------

In the Windows opentrack (the bridge):

- input: UDP over network, port 4242
- output: FreeTrack 2.0 Enhanced, settings: interface = TrackIR, plain freetrack unchecked
- filter: empty, filtering happened on the mac side

In the macOS opentrack:

- tracker: neuralnet (or any webcam tracker)
- output: UDP over network, 127.0.0.1, port 4242

The profile files of Windows opentrack live in the bottle at drive_c/users/<you>/Documents/opentrack-2.3 and this is all of it:

    [modules]
    filter-dll=
    protocol-dll=freetrack
    tracker-dll=udp

    [proto-freetrack]
    used-interfaces=2

Copy freetrackclient64.dll from modules/ into the game folder next to the game exe, renamed to FreeTrackClient.dll. The game looks for that literal name.

Launch order matters: start tracking on both sides, then launch the game. The game only hooks tracking from the cockpit view, so if it stays unconnected, quit the game and relaunch it with tracking already running.

Part 5: running without the GUI
-------------------------------

Recent opentrack also has a --headless mode (issue #2237, PR #2238): opentrack --headless --profile NAME starts tracking with no window and exits on SIGTERM. Under CrossOver, pass the platform as an argument rather than an environment variable, because CrossOver scrubs the environment: opentrack.exe --headless -platform offscreen. The offscreen platform plugin ships in platforms/ of the Qt you deployed.

At the time of writing the headless path under CrossOver is still less stable than the GUI (silent exits, no console output to debug with), so the GUI bridge is the dependable choice today. On Linux the same --headless mode combined with QT_QPA_PLATFORM=offscreen runs fine without any display at all.

Part 6: troubleshooting
-----------------------

- icuuc.dll not found at startup: that is an official opentrack build. Use a MinGW build per part 2.
- wine aborts with unimplemented function mscms.dll.WcsCreateIccProfile: apply part 3.
- undefined reference to __imp___argc when linking: your mingw toolchain is UCRT-flavored while Qt's entry point library is msvcrt-flavored. Either use llvm-mingw or keep your own WinMain (the source tree defines one for MinGW since this change).
- opentrack.exe --headless exits instantly with no output: GUI-subsystem programs under wine often lose stderr. Check the GUI once to confirm the configuration works, then use the same profile headless.
- no cameras in the Windows opentrack: expected, see the start of this document. Track the camera on the macOS side.
- the game ignores tracking: FreeTrackClient.dll missing or misnamed in the game folder, TrackIR interface not selected in the FreeTrack output settings, or the game was started before tracking.
