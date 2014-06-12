=======
Windows binary builds are available at <https://www.dropbox.com/sh/544fbhsokdpy3n7/AAAKwl6BluqwT9Xn2slyp0dCa>

Source code access available at <http://github.com/opentrack/opentrack/>

--

**opentrack** is an application dedicated to tracking user's head
movements and relaying them to games and flight simulation software.

Not to be confused with railway planning software <<http://opentrack.ch>>

See our wiki at <<https://github.com/opentrack/opentrack/wiki>>

# Tracking sources

- PointTracker by Patrick Ruoff, freetrack-like light sources
- Oculus Rift
- AR marker support via the ArUco library <https://github.com/rmsalinas/aruco>
- HT tracker <https://github.com/sthalik/headtracker>
- Razer Hydra
- Relaying via UDP from a different computer
- Joystick analog axes (Windows only)

# Output

- FlightGear Nasal script
- FSUIPC for Microsoft Flight Simulator (Windows)
- SimConnect for newer Microsoft Flight Simulator (Windows)
- freetrack emulation (Windows)
- Relaying udp to another computer
- Joystick support via freedesktop.org libevdev (Linux)
- Joystick support via VJoy (Windows)
- Wine freetrack glue protocol (Linux, OSX)
- Tablet-like coordinate output (Windows)

# Configuration

**opentrack** offers output shaping, filtering, is buildable on
MS Windows, MacOSX and GNU/Linux.

Don't be afraid to submit an issue/feature request if the need arises.

# Credits

- Stanisław Halik
- Chris Thompson (aka mm0zct)
- Donovan Baarda
- Ryan Spicer (OSX tester, contributor)
- Patrick Ruoff (PT tracker)
- FuraX49 (hatire arduino tracker)
- Ulf Schreiber (PT tracker)
- uglyDwarf (high CON)
- George Trigonakis (tester)
- Wim Vriend (historically)
- Ron Hendriks (historically)

# Licensing information

The code originally licensed under GPLv3, new code is required to be
compatible with it unless resides in separate address space.

It's recommended to submit new code under ISC license, it's a shorter
boilerplate header than MIT/X11 or new BSD.
