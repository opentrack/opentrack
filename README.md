Windows binary builds are available at <http://ananke.laggy.pk/opentrack/>

Source code access available at <http://github.com/opentrack/opentrack/>

--

**opentrack** is an application dedicated to tracking user's head
movements and relaying them to games and flight simulation software.

# Tracking sources

- SeeingMachines FaceAPI <http://seeingmachines.com/> (Windows)
- PointTracker by Patrick Ruoff, freetrack-like light sources
- Oculus Rift (Windows; Linux testers welcome!)
- AR marker support via the ArUco library <https://github.com/rmsalinas/aruco>
- HT tracker <https://github.com/sthalik/headtracker>
- Razer Hydra (Windows only)
- Relaying via UDP from a different computer

# Output

- FlightGear Nasal script
- FSUIPC for Microsoft FSX (Windows)
- freetrack emulation (Windows)
- Relaying udp to another computer
- Joystick support via freedesktop.org libevdev (Linux)
- Joystick support via VJoy (Windows)
- Wine freetrack glue protocol (Linux)
- Tablet-like coordinate output (Windows)

# Configuration

**opentrack** offers output shaping, filtering, is buildable on
both MS Windows and GNU/Linux.

Don't be afraid to submit an issue/feature request if the need arises.

# Credits

- Stanisław Halik
- Chris Thompson (aka mm0zct)
- Donovan Baarda
- Patrick Ruoff (merging)
- Wim Vriend (historically)
- Ron Hendriks (historically)

# Licensing information

The code originally licensed under GPLv3, new code is required to be
compatible with it unless resides in separate address space.

It's recommended to submit new code under ISC license, it's a shorter
boilerplate header than MIT/X11 or new BSD.
