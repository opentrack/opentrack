[<img src="https://github.com/opentrack/opentrack/actions/workflows/cmake.yml/badge.svg">](https://github.com/opentrack/opentrack/actions/workflows/cmake.yml)

## Intro

opentrack is a program for tracking user's head rotation and transmitting it to flight simulation software and military-themed video games. Project home is located at <<https://github.com/opentrack/opentrack>>.

Looking for **railway planning software**? <<https://opentrack.ch>> had the name `opentrack` first. Apologies for the long-standing naming conflict.

For the latest **downloads** visit <<https://github.com/opentrack/opentrack/releases>> Download an `.exe` installer or a `.7z` archive. Currently installers and portable versions for Windows are available for each release. It supports [USB stick truly "portable" installations](https://github.com/opentrack/opentrack/wiki/portable-mode-for-USB-sticks)

Please first refer to <<https://github.com/opentrack/opentrack/wiki>>
for [new user guide](https://github.com/opentrack/opentrack/wiki/Quick-Start-Guide-(WIP)), [frequent answers](https://github.com/opentrack/opentrack/wiki/common-issues), specific tracker/filter
documentation. See also the [gameplay video](https://www.youtube.com/watch?v=XI73ul_FnBI) with opentrack set up.

## Usage

`opentrack` is an application dedicated to tracking user's head
movements and relaying the information to games and flight simulation
software.

`opentrack` allows for output shaping, filtering, and operating with many input and output devices and protocols; the codebase runs Microsoft Windows, Apple OSX (currently unmaintained), and GNU/Linux.

Don't be afraid to submit an **issue/feature request** if you have any problems! We're a friendly bunch.

## Tracking input

- PointTracker by Patrick Ruoff, FreeTrack-like light points
- Oculus Rift (Windows only)
- Paper [marker](https://github.com/opentrack/opentrack/wiki/Aruco-tracker) via the Aruco<sup>[[1](https://github.com/opentrack/aruco)]</sup> library
- Razer Hydra
- Relaying via UDP from a different computer
- Relaying UDP via the FreePIE<sup>[[1](https://andersmalmgren.github.io/FreePIE/)]</sup> Android [apps](https://github.com/opentrack/opentrack/tree/master/contrib/freepie-udp)
- Joystick analog axes (Windows)
- Windows Phone [tracker](https://github.com/ZanderAdam/OpenTrack.WindowsPhone/wiki) over opentrack UDP protocol
- Arduino with custom Hatire firmware
- Intel RealSense 3D camera (Windows)
- BBC micro:bit, LEGO, sensortag support via Smalltalk<sup>[(1)](https://en.wikipedia.org/wiki/Smalltalk)[(2)](https://en.wikipedia.org/wiki/Alan_Kay)</sup>
  [S2Bot](https://www.picaxe.com/Teaching/Other-Software/Scratch-Helper-Apps/)
- Wiimote (Windows)
- NeuralNet face tracker
- Eyeware Beam<sup>[[1](https://beam.eyeware.tech/)]</sup>
- Tobii eye tracker

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
- Wim Vriend -- author of [FaceTrackNoIR](https://facetracknoir.sourceforge.net/) that served as the initial codebase for `opentrack`. While the  code was almost entirely rewritten, we still hold on to many of `FaceTrackNoIR`'s ideas.
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
- Davide Mameli (Eyeware Beam tracker)
- Khoa Nguyen (Tobii eye tracker)

## Thanks

- uglyDwarf (of [linuxtrack](https://github.com/uglyDwarf/linuxtrack/))
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

To edit the wiki, send pull requests to the [opentrack/wiki](https://github.com/opentrack/wiki) repository. The [user-facing wiki](https://github.com/opentrack/opentrack/wiki) will automatically update itself once the commit is merged.

## License and warranty

Almost all code is licensed under the [ISC license](https://en.wikipedia.org/wiki/ISC_license). There are very few proprietary dependencies. There is no copyleft code. See individual files for licensing and authorship information.

See [WARRANTY.txt](WARRANTY.txt) for applying warranty terms (that is, disclaiming possible pre-existing warranty) that are in force unless the software author specifies their own warranty terms. In short, we disclaim all possible warranty and aren't responsible for any possible damage or losses.

The code is held to a high-quality standard and written with utmost care; consider this a promise without legal value. Despite doing the best we can not to injure users' equipment, software developers don't want to be dragged to courts for imagined or real issues. Disclaiming warranty is a standard practice in the field, even for expensive software like operating systems.

## Building opentrack from source

On Windows, use either mingw-w64 or MS Visual Studio 2015 Update 3/newer. On other platforms use GNU or LLVM. Refer to [Visual C++ 2015 build instructions](https://github.com/opentrack/opentrack/wiki/Building-under-MS-Visual-C---2017-and-later).

On Linux, see [our wiki's Linux build instructions](https://github.com/opentrack/opentrack/wiki/Building-on-Linux).

