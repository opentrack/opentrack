This project was derived from the 'Sweetspotter' project.


If you want to work with the source code you should use the Microsoft Visual Studio 2008 and the Qt 4.6.x or better source code. (http://qt.nokia.com/downloads) Also it's useful to install the Qt Visual Studio Add-In.

The integration of Qt (LGPL) in Visual Studio must be done manually. See the PDF's in the documentation folder for specific instructions. 

Here are some remarks, which were not mentioned in the PDF's, but have to be followed:
Remark 1: instead of just running "configure" you should run 
"configure -debug-and-release -no-qt3support -no-webkit -platform win32-msvc2008".

Remark 2: in the Qt folders (\src\script\tmp\moc\debug_shared\, src\script\tmp\moc\release_shared\
and src\3rdparty\webkit\WebCore\tmp\moc can be a file called: mocinclude.tmp. Remove these, because "nmake" will fail if you don't.


You need also:
- FaceApi www.faceapi.com

You need to setup the folders to the binaries and include files in the Visual Studio IDE.

	Tools > Options > Projects and Solutions > VC++ Directories

Example in the standard way:

	Executable Files:
	
		C:\Program Files (x86)\SeeingMachines\FaceTrackingAPI_NC 3.1\API\bin
		
	Include Files:
	
		C:\Program Files (x86)\SeeingMachines\FaceTrackingAPI_NC 3.1\API\include
		C:\Program Files (x86)\SeeingMachines\FaceTrackingAPI_NC 3.1\Wrappers\C++\include
		C:\Program Files (x86)\SeeingMachines\FaceTrackingAPI_NC 3.1\Wrappers\Qt\include
		C:\Program Files (x86)\FMOD SoundSystem\FMOD Programmers API Win32\api\inc
		C:\Qt\4.6.2\include
	
	Library Files:
	
		C:\Program Files (x86)\SeeingMachines\FaceTrackingAPI_NC 3.1\Wrappers\Qt\lib
		C:\Program Files (x86)\SeeingMachines\FaceTrackingAPI_NC 3.1\Wrappers\C++\lib
		C:\Program Files (x86)\SeeingMachines\FaceTrackingAPI_NC 3.1\API\bin
		C:\Program Files (x86)\FMOD SoundSystem\FMOD Programmers API Win32\api\lib
		C:\Qt\4.6.2\lib


Best regards,

Wim Vriend.
