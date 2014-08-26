// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the VJOYINTERFACE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// VJOYINTERFACE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef VJOYINTERFACE_EXPORTS
#define VJOYINTERFACE_API __declspec(dllexport)
#else
#define VJOYINTERFACE_API __declspec(dllimport)
#endif

///////////////////////////// vJoy device (collection) status ////////////////////////////////////////////
#ifndef VJDSTAT
#define VJDSTAT
enum VjdStat  /* Declares an enumeration data type called BOOLEAN */
{
	VJD_STAT_OWN,	// The  vJoy Device is owned by this application.
	VJD_STAT_FREE,	// The  vJoy Device is NOT owned by any application (including this one).
	VJD_STAT_BUSY,	// The  vJoy Device is owned by another application. It cannot be acquired by this application.
	VJD_STAT_MISS,	// The  vJoy Device is missing. It either does not exist or the driver is down.
	VJD_STAT_UNKN	// Unknown
}; 
#endif
extern "C" {
///////////////////////////// vJoy device (collection) Control interface /////////////////////////////////
/*
	These functions allow writing feeders and other applications that interface with vJoy
	It is assumed that only one vJoy top-device (= Raw PDO) exists.
	This top-level device can have up to 16 siblings (=top-level Reports/collections)
	Each sibling is refered to as a "vJoy Device" and is attributed a unique Report ID (Range: 1-16).

	Naming convetion:
		VJD = vJoy Device
		rID = Report ID
*/

/////	General driver data
VJOYINTERFACE_API SHORT __cdecl GetvJoyVersion(void);
VJOYINTERFACE_API BOOL	__cdecl vJoyEnabled(void);
VJOYINTERFACE_API PVOID	__cdecl	GetvJoyProductString(void);
VJOYINTERFACE_API PVOID	__cdecl	GetvJoyManufacturerString(void);
VJOYINTERFACE_API PVOID	__cdecl	GetvJoySerialNumberString(void);


/////	vJoy Device properties
VJOYINTERFACE_API int	__cdecl  GetVJDButtonNumber(UINT rID);	// Get the number of buttons defined in the specified VDJ
VJOYINTERFACE_API int	__cdecl  GetVJDDiscPovNumber(UINT rID);	// Get the number of descrete-type POV hats defined in the specified VDJ
VJOYINTERFACE_API int	__cdecl  GetVJDContPovNumber(UINT rID);	// Get the number of descrete-type POV hats defined in the specified VDJ
VJOYINTERFACE_API BOOL	__cdecl  GetVJDAxisExist(UINT rID, UINT Axis); // Test if given axis defined in the specified VDJ
VJOYINTERFACE_API BOOL	__cdecl  GetVJDAxisMax(UINT rID, UINT Axis, LONG * Max); // Get logical Maximum value for a given axis defined in the specified VDJ
VJOYINTERFACE_API BOOL	__cdecl  GetVJDAxisMin(UINT rID, UINT Axis, LONG * Min); // Get logical Minimum value for a given axis defined in the specified VDJ

/////	Write access to vJoy Device - Basic
VJOYINTERFACE_API BOOL		__cdecl	AcquireVJD(UINT rID);				// Acquire the specified vJoy Device.
VJOYINTERFACE_API VOID		__cdecl	RelinquishVJD(UINT rID);			// Relinquish the specified vJoy Device.
VJOYINTERFACE_API BOOL		__cdecl	UpdateVJD(UINT rID, PVOID pData);	// Update the position data of the specified vJoy Device.
VJOYINTERFACE_API enum VjdStat	__cdecl	GetVJDStatus(UINT rID);			// Get the status of the specified vJoy Device.

/////	Write access to vJoy Device - Modifyiers
// This group of functions modify the current value of the position data
// They replace the need to create a structure of position data then call UpdateVJD

//// Reset functions
VJOYINTERFACE_API BOOL		__cdecl	ResetVJD(UINT rID);			// Reset all controls to predefined values in the specified VDJ
VJOYINTERFACE_API VOID		__cdecl	ResetAll(void);				// Reset all controls to predefined values in all VDJ
VJOYINTERFACE_API BOOL		__cdecl	ResetButtons(UINT rID);		// Reset all buttons (To 0) in the specified VDJ
VJOYINTERFACE_API BOOL		__cdecl	ResetPovs(UINT rID);		// Reset all POV Switches (To -1) in the specified VDJ

// Write data
VJOYINTERFACE_API BOOL		__cdecl	SetAxis(LONG Value, UINT rID, UINT Axis);		// Write Value to a given axis defined in the specified VDJ 
VJOYINTERFACE_API BOOL		__cdecl	SetBtn(BOOL Value, UINT rID, UCHAR nBtn);		// Write Value to a given button defined in the specified VDJ 
VJOYINTERFACE_API BOOL		__cdecl	SetDiscPov(int Value, UINT rID, UCHAR nPov);	// Write Value to a given descrete POV defined in the specified VDJ 
VJOYINTERFACE_API BOOL		__cdecl	SetContPov(DWORD Value, UINT rID, UCHAR nPov);	// Write Value to a given continuous POV defined in the specified VDJ 

} // extern "C"