FaceTrackNoIR (v. 20100615).

From various users we received requests for improvements and features. If you keep sending us your ideas, we can keep
improving FaceTrackNoIR...

Installation:
Unzip the .exe to the installation folder of FaceTrackNoIR (best rename the old .exe first). The folders in the ZIP-file
contain support info and examples of a .INI-file (IL-2) and a script for GlovePIE.


The following upgrades have been applied:

-	‘=’ is center view 

-	‘BACKSPACE’ = start (+center)/stop tracking. A messagebeep is generated when tracking is resumed and the
		      headpose-data is valid again (faceAPI has 'locked on'... ).
	
-	Checkbox ‘use EWMA filter’: Ticking this checkbox will make FaceTrackNoIR filter the headpose data with
	a so-called 'Exponentially Weighed Moving Average'. This technique was adopted from FlightGear, where
	Melchior Franz had 'invented' it. 
	If the filter is active, the factor for each of the 6 DOF's are used. These factors determine the weight that is 
	given to previous measurements and goes from 0 - 1 (in the GUI this setting is 0 - 100). A lower value will give a 	quicker response (but less stable).

	Remark: for FlightGear, the EWMA checkbox does nothing: the filter is embedded in the FlightGear script.

-	Some timing-issues with the faceAPI were resolved, so now the scanning-frequency can be higher that 20Hz.
	We do not know yet, how this influences performance of both FaceTrackNoIR and your CPU. Experience learns, that
	the headtracking is influenced by many factors, like CPU, video-card, framerate of the game etc.

-	The protocol-server that is selected in the combobox is now the only one that is started, when the tracker is started.
	Before loading an .INI-file, please stop the tracker.

-	PPJoy support is added, so now FaceTrackNoIR can be used for other games too. We tested this feature with IL-2,
	using GlovePIE to generate TrackIR data. Check out the files in the ZIP-file for instructions. PPjoy was created by
	Deon van der Westhuysen and GlovePIE by Carl Kenner. We thank them both for their effort!



Please let us know what you think of our little gadget. Posts can be made on the FaceTrackNoIR forum on SourceForge 
(http://sourceforge.net/projects/facetracknoir/forums/forum/1150910) or on the various game-forums.

If anyone has used FaceTrackNoIR successfully with other games, please let us know. If you can, we would also like to receive
examples of the .INI-file you used for that and maybe settings of other utilities needed. Thanks!



The FaceTrackNoIR team:

Wim Vriend
Ron Hendriks

