FaceTrackNoIR (v. 1.0.0).

FaceTrackNoIR is a head-tracker, which uses the FaceAPI provided by SeeingMachines. It was made using Visual Studio 2008 and Qt.The major advantage over other headtrackers is, that it uses a simple webcam to track the face of 'the gamer'. There is no need for expensive equipment or even Borg-like devices with LED's and such.


Installation:
To install the program, simply start Setup.exe and follow the directions. Use the desktop-icon to start FaceTrackNoIR.

Compatibility:
FaceTrackNoIR is made for Windows and tested on XP, Vista and Windows7. The FaceAPI creators recommend a dual-core processor or better. Because the 'non-commercial' version of the FaceAPI is used, the webcam can not be 'chosen': it always uses the first webcam it finds! When FaceTrackNoIR is started, the name of this webcam is displayed.


Games:
Until now, FaceTrackNoIR supports the protocol created by Free-track (ArmA 2 supports this) and FlightGear. We are working on others...

Testing:
To test the Free-track protocol, we included FreeTrackTest.exe, which was made by the Free-track team. If this works, ArmA 2 also will.

Settings:
The head-tracking settings can be loaded and saved, using the menu-items under 'File'. The last settings will automatically load on startup. It may be necessary to fiddle with these settings: the provided .ini files are not really optimized!

Remarks:
- To reduce CPU-load it is best to minimize FaceTrackNoIR before starting/switching to the game.
- The combobox 'Game protocol' shows 3 choices, however it does not actually do anything with the selection (yet). Both Free-track and FlightGear protocols are started.
- We are planning to make FaceTrackNoIR 'server-client' modes. This way the head-tracking can be run on one computer and the game on another. This should significantly improve performance.

 

Please let us know if you like the program, if you have ideas for improvements or any questions you might have.
The source is also available!


The FaceTrackNoIR team:

Wim Vriend
Ron Hendriks

