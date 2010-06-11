FaceTrackNoIR (v. 1.0.1).

The following upgrades have been applied:

	‘=’ is center view 

	‘BACKSPACE’ = start (+center)/stop tracking. A messagebeep is generated when tracking is resumed and the
		      headpose-data is valid again (faceAPI has 'locked on'... 
	
	Checkbox ‘use EWMA filter’: Ticking this checkbox will make FaceTrackNoIR filter the raw data with
	a so-called 'Exponentially Weighed Moving Average'. This technique was adopted from FlightGear, where
	Melchior Franz had 'invented' it. If the filter is active, the factor for each of the 6 DOF's are used.
	These factor determine the weight that is given to previous measurements and goes from 0 - 1 (in the GUI
	this setting is 0 - 100).

	Some timing-issues with the faceAPI were resolved, so now the scanning-frequency can be higher that 20Hz.
	We do not know yet, how this influences performance of both FaceTrackNoIR and your CPU.



Please let us know what you think of our little gadget. Posts can be made on the FaceTrackNoIR forum on SourceForge 
(http://sourceforge.net/projects/facetracknoir/forums/forum/1150910) or on the various game-forums.



The FaceTrackNoIR team:

Wim Vriend
Ron Hendriks

