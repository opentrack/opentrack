# This Fork

This is a fork of the open source project [opentrack](https://github.com/opentrack/opentrack) aiming to improve compatibility with the game [Star Citizen](https://robertsspaceindustries.com/) on Linux.

## Changes:

 - The ability to select your own wine exectuable for the wine output module since SC players might change runners and are not using system wine but lutris instead.
 - Enabled to build the wine output module by default with CMake which is required to use headtracking in SC.
 - Added a current game label to the output group field, which displays the current game title. This is usually also shown in the opentrack window title but just to make it more noticalbe it is also now shown there.
 - Fixed a issue with the wine output module that caused SC to no longer start TrackIR correctly until a registry key for both freetrack AND NPClient were created. (DEFAULT: Only one gets created for either option, unless the `Both` protocol is selected.)

## Installation:

 1. Clone this repository: `git clone https://github.com/Priton-CE/opentrack-StarCitizen.git` and cd into the new directory
 2. Create a build directory: `mkdir build`
 3. Move into the directory: `cd build`
 4. Run CMake: `cmake ..`
 5. Build opentrack: `make`
 6. Install opentrack to the install directory: `make install`
 (you can find opentrack in `install/bin/opentrack` and the updated modules in `install/libexec/opentrack/`)

Visit the [LUG Wiki](https://github.com/starcitizen-lug/information-howtos/wiki) for further deatils about the game on Linux.

## Usage:

 1. Open Lutris and check your selected runner, prefix, esync and fsync settings
 2. Open Opentrack-SC and select the wine module as the output module
 3. Configure the wine module by pressing the configure button next to it
 4. Select your runner via the dropdown.
 5. Type the path to your wine prefix into the second field (prefer absolute paths as well like: `/home/$USER/Games/star-citizen/`) or use the browse button.
 6. Make sure that your esync and fsync settings match the ones in Lutris
 7. Select `Freetrack` as the output (NOTE: `Both` is required if normal Opentrack is used. This fork has been patched so that `Freetrack` works too.)
 8. Launch Star Citizen and navigate to the `COMMS, FOIP & HEAD TRACKING` settings
 9. Set `Head Tracking - General - Source` to `TrackIR`
 10. Set `Head Tracking - General - Toggle - Enabled` to `Yes`
 11. Adjust other `Head Tracking - General - *` settings to your liking

## Known Issues:
I should have fixed all other issues with Star Citizen and Opentrack I found using both myself.
If you still have issues with Star Citizen not starting or using TrackIR correctly check the registry `Software/Freetrack/FreeTrackClient` and `Software/NaturalPoint/NATURALPOINT/NPClient Location` for the keys called `Path` and make sure both have a value leading to your opentrack installation. Alternatively open an issue on GitHub or join the [Star Citizen LUG Discord or Matrix](https://github.com/starcitizen-lug/knowledge-base#socials).

# opentrack README

The rest of the opentrack README.md file:

## Intro

[<img src="https://ci.appveyor.com/api/projects/status/n0j9h38jnif5qbe9/branch/unstable?svg=true"/>](https://ci.appveyor.com/project/sthalik/opentrack/branch/unstable)

opentrack project home is located at <<http://github.com/opentrack/opentrack>>.

For the latest **downloads** visit <<https://github.com/opentrack/opentrack/releases>> Download an `.exe` installer or a `.7z` archive. Currently installers and portable versions for Windows are available for each release. It supports [USB stick truly "portable" installations](https://github.com/opentrack/opentrack/wiki/portable-mode-for-USB-sticks)

Please first refer to <<https://github.com/opentrack/opentrack/wiki>>
for [new user guide](https://github.com/opentrack/opentrack/wiki/Quick-Start-Guide-(WIP)), [frequent answers](https://github.com/opentrack/opentrack/wiki/common-issues), specific tracker/filter
documentation. See also the [gameplay video](https://www.youtube.com/watch?v=XI73ul_FnBI) with opentrack set up.

## Looking for railway planning software?

**Railway planning software** <<http://opentrack.ch>> had the name `opentrack` first. Apologies for the long-standing naming conflict.

## Usage

`opentrack` is an application dedicated to tracking user's head
movements and relaying the information to games and flight simulation
software.

`opentrack` allows for output shaping, filtering, and operating with many input and output devices and protocols; the codebase runs Microsoft Windows, Apple OSX (currently unmaintained), and GNU/Linux.

Don't be afraid to submit an **issue/feature request** if you have any problems! We're a friendly bunch.

## Tracking input

- PointTracker by Patrick Ruoff, freetrack-like light sources
- Oculus Rift DK1, DK2, CV, and legacy/knockoff versions (Windows only)
- Paper [marker support](https://github.com/opentrack/opentrack/wiki/Aruco-tracker)
  via the ArUco library <<https://github.com/opentrack/aruco>>
- Razer Hydra
- Relaying via UDP from a different computer
- Relaying UDP via FreePIE-specific Android app
- Joystick analog axes (Windows)
- Windows Phone [tracker](https://github.com/ZanderAdam/OpenTrack.WindowsPhone/wiki) over opentrack UDP protocol
- Arduino with custom firmware
- Intel RealSense 3D cameras (Windows)
- BBC micro:bit, LEGO, sensortag support via Smalltalk<sup>[(1)](https://en.wikipedia.org/wiki/Smalltalk)[(2)](https://en.wikipedia.org/wiki/Alan_Kay)</sup>
  [S2Bot](http://www.picaxe.com/Teaching/Other-Software/Scratch-Helper-Apps/)
- Wiimote (Windows)

## Output protocols

- SimConnect for newer Microsoft Flight Simulator (Windows)
- freetrack implementation (Windows)
- Relaying UDP to another computer
- Virtual joystick output (Windows, Linux, OSX)
- Wine freetrack glue protocol (Linux, OSX)
- X-Plane plugin (Linux; uses the Wine output option)
- Tablet-like mouse output (Windows)
- FlightGear
- FSUIPC for Microsoft Flight Simulator 2002/2004 (Windows)
- SteamVR through a bridge (Windows; see <<https://github.com/r57zone/OpenVR-OpenTrack>> by @r57zone)

## Credits, in chronological order

- Stanisław Halik (maintainer)
- Wim Vriend -- author of [FaceTrackNoIR](http://facetracknoir.sourceforge.net/) that served as the initial codebase for `opentrack`. While the  code was almost entirely rewritten, we still hold on to many of `FaceTrackNoIR`'s ideas.
- Chris Thompson (aka mm0zct, Rift and Razer Hydra author and maintainer)
- Patrick Ruoff (PT tracker author)
- Xavier Hallade (Intel RealSense tracker author and maintainer)
- furax49 (hatire tracker author)
- Michael Welter (contributor)
- Alexander Orokhovatskiy (Russian translation; profile repository maintenance; providing hardware; translating reports from the Russian community)
- Attila Csipa (Micro:Bit author)
- Eike "e4z9" (OSX joystick output driver)
- Wei Shuai (Wiimote tracker)
- Stéphane Lenclud (Kinect Face Tracker, Easy Tracker)
- GO63-samara (Hamilton Filter, Pose-widget improvement)

## Thanks

- uglyDwarf (high CON)
- Andrzej Czarnowski (FreePIE tracker and
  [Google Cardboard](https://github.com/opentrack/opentrack/wiki/VR-HMD-goggles-setup-----google-cardboard,-colorcross,-opendive)
  assistance, testing)
- Wim Vriend (original codebase author and maintainer)
- Ryan Spicer (OSX tester, contributor)
- Ries van Twisk (OSX tester, OSX Build Fixes, contributor)
- Donovan Baarda (filtering/control theory expert)
- Mathijs Groothuis (@MathijsG, dozens of bugs and other issues reported; NL translation)
- The Russian community from the [IL-2 Sturmovik forums](https://forum.il2sturmovik.ru/) (reporting bugs, requesting important features)
- OpenCV authors and maintainers <<https://github.com/opencv/opencv/>>.

## Contributing

See guides for writing new modules\[[1](https://github.com/opentrack/opentrack/blob/master/api/plugin-api.hpp)\]\[[2](https://github.com/opentrack/opentrack/blob/master/tracker-test/test.h)\], and for [working with core code](https://github.com/opentrack/opentrack/wiki/Hacking-opentrack).

## License and warranty

Almost all code is licensed under the [ISC license](https://en.wikipedia.org/wiki/ISC_license). There are very few proprietary dependencies. There is no copyleft code. See individual files for licensing and authorship information.

See [WARRANTY.txt](WARRANTY.txt) for applying warranty terms (that is, disclaiming possible pre-existing warranty) that are in force unless the software author specifies their own warranty terms. In short, we disclaim all possible warranty and aren't responsible for any possible damage or losses.

The code is held to a high-quality standard and written with utmost care; consider this a promise without legal value. Despite doing the best we can not to injure users' equipment, software developers don't want to be dragged to courts for imagined or real issues. Disclaiming warranty is a standard practice in the field, even for expensive software like operating systems.

## Building opentrack from source

On Windows, use either mingw-w64 or MS Visual Studio 2015 Update 3/newer. On other platforms use GNU or LLVM. Refer to [Visual C++ 2015 build instructions](https://github.com/opentrack/opentrack/wiki/Building-under-MS-Visual-C---2017-and-later).
