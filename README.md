Windows binary builds are available at <<https://www.dropbox.com/sh/544fbhsokdpy3n7/AAAKwl6BluqwT9Xn2slyp0dCa>>

Source code access available at <<http://github.com/opentrack/opentrack/>>

Please first refer to <<https://github.com/opentrack/opentrack/wiki>>
for new user guide, frequent questions, specific tracker/filter
documentation.

***

**opentrack** is an application dedicated to tracking user's head
movements and relaying the information to games and flight simulation
software.

Not to be confused with railway planning software <<http://opentrack.ch>>

***

# Tracking sources

- PointTracker by Patrick Ruoff, freetrack-like light sources
- Oculus Rift
- Paper marker support via the ArUco library <<https://github.com/rmsalinas/aruco>>
- Human face tracker <<https://github.com/sthalik/headtracker>>
- Razer Hydra
- Relaying via UDP from a different computer
- Relaying UDP via FreePIE-specific Android app
- Joystick analog axes (Windows, Linux)

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

Don't be afraid to submit an issue/feature request if the need arises.

***

# Credits

- Stanisław Halik (maintainer)
- Chris Thompson (aka mm0zct)
- Donovan Baarda (filtering/control theory expert)
- Ryan Spicer (OSX tester, contributor)
- Patrick Ruoff (PT tracker)
- Ulf Schreiber (PT tracker)
- uglyDwarf (high CON)
- Wim Vriend (historically)

# Licensing information

The code originally licensed under GPLv3, new code required legally
compatible unless resides in separate address space.

Recommended to submit new code under ISC license, shorter boilerplate
header than MIT/X11 or new BSD.
