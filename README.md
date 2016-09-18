opentrack project home at <<http://github.com/opentrack/opentrack>>.

Downloads are at <<https://github.com/opentrack/opentrack/releases>>.

Please first refer to <<https://github.com/opentrack/opentrack/wiki>>
for new user guide, frequent questions, specific tracker/filter
documentation.

***

**opentrack** is an application dedicated to tracking user's head
movements and relaying the information to games and flight simulation
software.

Not to be confused with railway planning software <<http://opentrack.ch>>

***

# Downloads

For the latest releases visit: <<https://github.com/opentrack/opentrack/releases>> . You will find the software there as well as the source code.

# Tracking sources

- PointTracker by Patrick Ruoff, freetrack-like light sources
- Oculus Rift DK1, DK2 and legacy/knockoff versions (Windows only)
- Paper marker support via the ArUco library <<https://github.com/rmsalinas/aruco>>
- Razer Hydra
- Relaying via UDP from a different computer
- Relaying UDP via FreePIE-specific Android app
- Joystick analog axes (Windows, Linux)
- Windows Phone [tracker](http://www.windowsphone.com/en-us/store/app/opentrack-head-tracking/1c604f32-6d68-40ef-aa44-3163e30f547f) over opentrack UDP protocol
- Arduino with custom firmware
- Intel RealSense 3D cameras (Windows)

# Output

- SimConnect for newer Microsoft Flight Simulator (Windows)
- freetrack implementation (Windows)
- Relaying UDP to another computer
- Virtual joystick output (Linux, Windows)
- Wine freetrack glue protocol (Linux, OSX)
- X-Plane plugin (Linux)
- Tablet-like mouse output (Windows)
- FlightGear Nasal script
- FSUIPC for Microsoft Flight Simulator 2002/2004 (Windows)

***

# Configuration

**opentrack** allows for output shaping, filtering, the codebase builds
on on Microsoft Windows, Apple OSX, and GNU/Linux.

Don't be afraid to submit an issue/feature request if need arises.

***

# Credits

- Stanisław Halik (maintainer)
- Chris Thompson (aka mm0zct, Rift and Razer Hydra author and maintainer)
- Patrick Ruoff (PT tracker author)
- Xavier Hallade (Intel RealSense tracker author and maintainer)
- furax49 (hatire tracker author)
- Michael Welter (contributor)

# Thanks

- uglyDwarf (high CON)
- Andrzej Czarnowski (FreePIE tracker and Google Cardboard assistance, testing)
- Wim Vriend (original codebase author and maintainer)
- Ryan Spicer (OSX tester, contributor)
- Donovan Baarda (filtering/control theory expert)
- Mathijs Groothuis (@MathijsG, dozens of bugs and other issues reported)

# Licensing information

Almost all code is licensed under the [ISC license](https://en.wikipedia.org/wiki/ISC_license). There are very few proprietary dependencies. There is no copyleft code. See individual files for licensing and authorship information.

# Warranty terms

See [WARRANTY.TXT](WARRANTY.txt) for applying warranty terms (that is, disclaiming possible pre-existing warranty) that are in force unless the software author specifies their own warranty terms.

# Building opentrack from source

On Windows, use either mingw-w64 or MS Visual Studio 2015 Update 3/newer. On other platforms use GNU or LLVM. Refer to [Visual C++ 2015 build instructions](https://github.com/opentrack/opentrack/wiki/Building-under-MS-Visual-C------2015).
