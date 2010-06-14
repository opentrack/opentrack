//**********************************************************************
//	Filename:	TrackIR.c
//	Authors:	Wolfram "Osram" Kuss (original)
//				Lukas "Retro" Friembichler (adapted for EECH)
//	Date:		26. Feb 2003
//				26/09/03 adapted for falcon - SHOCK, HORROR, FALCONEERS GET (TWICE !!!) RECYCLED STUFF... EEEEK
//	Update:
//
//	Description:Implements TrackIR support for EECH
//				Commented code is from the (C++) MiG/BoB Version..
//				This file was originally split into 2, with the API
//				originally written by
//				Doyle Nickless -- 13 Jan, 2003 -- for Eye Control Technology.
//*********************************************************************/
#include "stdhdr.h"

#include "TrackIR.h"
#include "OTWDrive.h"
#include "sinput.h"

//#define DEBUG_TRACKIR_STUFF 0

// Retro 02/10/03
//	enable TIR globally (tell the app to try and init it,
//	is also a status flag if init is succesful)
bool g_bEnableTrackIR = false;
bool g_bTrackIRon = false;

/* config vars declared in f4config.cpp */
extern int g_nTrackIRSampleFreq;
extern float g_fTIR2DYawPercentage;
extern float g_fTIR2DPitchPercentage;

// FIRST PART (originally NPClientWraps.c)

// *******************************************************************************
// *
// * Module Name:
// *   NPClientWraps.cpp
// *
// * Software Engineer:
// *   Doyle Nickless - GoFlight Inc., for Eye Control Technology.
// *
// * Abstract:
// *   This module implements the wrapper code for interfacing to the NaturalPoint
// *   Game Client API.  Developers of client apps can include this module into
// *   their projects to simplify communication with the NaturalPoint software.
// *
// *   This is necessary since the NPClient DLL is run-time linked rather than
// *   load-time linked, avoiding the need to link a static library into the
// *   client program (only this module is needed, and can be supplied in source
// *   form.)
// *
// * Environment:
// *   User mode
// *
// * Update:	Retro, Feb 2003 - Threw out the MFC stuff, made it compile in C
// *			Mostly messed around in NPClient_Init() though..
// *
// *			Retro 26/09/03 - moved to FalconLand =)
// *******************************************************************************

//////////////////
/// Defines //////////////////////////////////////////////////////////////////////
/////////////////
#define         VERSION_MAJOR           1
#define         VERSION_MINOR           0
#define         VERSION_BUILD           1

// magic to get the preprocessor to do what we want
#define		lita(arg) #arg
#define		xlita(arg) lita(arg)
#define		cat3(w,x,z) w##.##x##.##z##\000
#define		xcat3(w,x,z) cat3(w,x,z)
#define		VERSION_STRING xlita(xcat3(VERSION_MAJOR,VERSION_MINOR,VERSION_BUILD))

//
// Versioning hasn't been worked out yet...
//
// The following is the previous spec definition of versioning info -- I can probably do
// something very similar to this -- will keep you posted.
//
// request version information using 2 messages, they cannot be expected to arrive in a specific order - so always parse using the High byte
// the messages have a NPCONTROL byte in the first parameter, and the second parameter has packed bytes.
//   Message 1) (first parameter)NPCONTROL : (second parameter) (High Byte)NPVERSIONMAJOR (Low Byte) major version number data
//   Message 2) (first parameter)NPCONTROL : (second parameter) (High Byte)NPVERSIONMINOR (Low Byte) minor version number data

#define	NPQUERYVERSION	1040

// CONTROL DATA SUBFIELDS
#define	NPVERSIONMAJOR	1
#define	NPVERSIONMINOR	2

// DATA FIELDS
#define	NPControl		8	// indicates a control data field
						// the second parameter of a message bearing control data information contains a packed data format. 
						// The High byte indicates what the data is, and the Low byte contains the actual data
// roll, pitch, yaw
#define	NPRoll		1	// +/- 16383 (representing +/- 180) [data = input - 16383]
#define	NPPitch		2	// +/- 16383 (representing +/- 180) [data = input - 16383]
#define	NPYaw		4	// +/- 16383 (representing +/- 180) [data = input - 16383]

// x, y, z - remaining 6dof coordinates
#define	NPX			16	// +/- 16383 [data = input - 16383]
#define	NPY			32	// +/- 16383 [data = input - 16383]
#define	NPZ			64	// +/- 16383 [data = input - 16383]

// raw object position from imager
#define	NPRawX		128	// 0..25600 (actual value is multiplied x 100 to pass two decimal places of precision)  [data = input / 100]
#define	NPRawY		256  // 0..25600 (actual value is multiplied x 100 to pass two decimal places of precision)  [data = input / 100]
#define	NPRawZ		512  // 0..25600 (actual value is multiplied x 100 to pass two decimal places of precision)  [data = input / 100]

// x, y, z deltas from raw imager position 
#define	NPDeltaX		1024 // +/- 2560 (actual value is multiplied x 10 to pass two decimal places of precision)  [data = (input / 10) - 256]
#define	NPDeltaY		2048 // +/- 2560 (actual value is multiplied x 10 to pass two decimal places of precision)  [data = (input / 10) - 256]
#define	NPDeltaZ		4096 // +/- 2560 (actual value is multiplied x 10 to pass two decimal places of precision)  [data = (input / 10) - 256]

// raw object position from imager
#define	NPSmoothX		8192	  // 0..32766 (actual value is multiplied x 10 to pass one decimal place of precision) [data = input / 10]
#define	NPSmoothY		16384  // 0..32766 (actual value is multiplied x 10 to pass one decimal place of precision) [data = input / 10]
#define	NPSmoothZ		32768  // 0..32766 (actual value is multiplied x 10 to pass one decimal place of precision) [data = input / 10]


//////////////////
/// Typedefs /////////////////////////////////////////////////////////////////////
/////////////////

// NPESULT values are returned from the Game Client API functions.
//
typedef enum tagNPResult
{
	NP_OK = 0,
    NP_ERR_DEVICE_NOT_PRESENT,
	NP_ERR_UNSUPPORTED_OS,
	NP_ERR_INVALID_ARG,
	NP_ERR_DLL_NOT_FOUND,
	NP_ERR_NO_DATA,
	NP_ERR_INTERNAL_DATA
} NPRESULT;

typedef struct tagTrackIRData
{
	unsigned short wNPStatus;
	unsigned short wPFrameSignature;
	unsigned long  dwNPIOData;

	float fNPRoll;
	float fNPPitch;
	float fNPYaw;
	float fNPX;
	float fNPY;
	float fNPZ;
	float fNPRawX;
	float fNPRawY;
	float fNPRawZ;
	float fNPDeltaX;
	float fNPDeltaY;
	float fNPDeltaZ;
	float fNPSmoothX;
	float fNPSmoothY;
	float fNPSmoothZ;

} TRACKIRDATA, *LPTRACKIRDATA;

//
// Typedef for pointer to the notify callback function that is implemented within
// the client -- this function receives head tracker reports from the game client API
//
typedef NPRESULT (__stdcall *PF_NOTIFYCALLBACK)( unsigned short, unsigned short );

// Typedefs for game client API functions (useful for declaring pointers to these
// functions within the client for use during GetProcAddress() ops)
//
typedef NPRESULT (__stdcall *PF_NP_REGISTERWINDOWHANDLE)( HWND );
typedef NPRESULT (__stdcall *PF_NP_UNREGISTERWINDOWHANDLE)( void );
typedef NPRESULT (__stdcall *PF_NP_REGISTERPROGRAMPROFILEID)( unsigned short );
typedef NPRESULT (__stdcall *PF_NP_QUERYVERSION)( unsigned short* );
typedef NPRESULT (__stdcall *PF_NP_REQUESTDATA)( unsigned short );
typedef NPRESULT (__stdcall *PF_NP_GETDATA)( LPTRACKIRDATA );
typedef NPRESULT (__stdcall *PF_NP_REGISTERNOTIFY)( PF_NOTIFYCALLBACK );
typedef NPRESULT (__stdcall *PF_NP_UNREGISTERNOTIFY)( void );
typedef NPRESULT (__stdcall *PF_NP_STARTCURSOR)( void );
typedef NPRESULT (__stdcall *PF_NP_STOPCURSOR)( void );
typedef NPRESULT (__stdcall *PF_NP_STARTDATATRANSMISSION)( void );
typedef NPRESULT (__stdcall *PF_NP_STOPDATATRANSMISSION)( void );

//// Function Prototypes ///////////////////////////////////////////////
//
// Functions exported from game client API DLL ( note __stdcall calling convention
// is used for ease of interface to clients of differing implementations including
// C, C++, Pascal (Delphi) and VB. )
//
//NPRESULT __stdcall NP_RegisterWindowHandle( HWND hWnd );
NPRESULT __stdcall NP_RegisterWindowHandle( HWND );
NPRESULT __stdcall NP_UnregisterWindowHandle( void );
NPRESULT __stdcall NP_RegisterProgramProfileID( unsigned short wPPID );
NPRESULT __stdcall NP_QueryVersion( unsigned short* pwVersion );
NPRESULT __stdcall NP_RequestData( unsigned short wDataReq );
NPRESULT __stdcall NP_GetData( LPTRACKIRDATA pTID );
NPRESULT __stdcall NP_RegisterNotify( PF_NOTIFYCALLBACK pfNotify );
NPRESULT __stdcall NP_UnregisterNotify( void );
NPRESULT __stdcall NP_StartCursor( void );
NPRESULT __stdcall NP_StopCursor( void );
NPRESULT __stdcall NP_StartDataTransmission( void );
NPRESULT __stdcall NP_StopDataTransmission( void );

/////////////
// Defines ///////////////////////////////////////////////////////////////////////
/////////////
//

/////////////////
// Global Data ///////////////////////////////////////////////////////////////////
/////////////////
//
PF_NP_REGISTERWINDOWHANDLE       gpfNP_RegisterWindowHandle = NULL;
PF_NP_UNREGISTERWINDOWHANDLE     gpfNP_UnregisterWindowHandle = NULL;
PF_NP_REGISTERPROGRAMPROFILEID   gpfNP_RegisterProgramProfileID = NULL;
PF_NP_QUERYVERSION               gpfNP_QueryVersion = NULL;
PF_NP_REQUESTDATA                gpfNP_RequestData = NULL;
PF_NP_GETDATA                    gpfNP_GetData = NULL;
PF_NP_STARTCURSOR                gpfNP_StartCursor = NULL;
PF_NP_STOPCURSOR                 gpfNP_StopCursor = NULL;
PF_NP_STARTDATATRANSMISSION      gpfNP_StartDataTransmission = NULL;
PF_NP_STOPDATATRANSMISSION       gpfNP_StopDataTransmission = NULL;

HMODULE ghNPClientDLL = (HMODULE)NULL;

////////////////////////////////////////////////////
// NaturalPoint Game Client API function wrappers /////////////////////////////
////////////////////////////////////////////////////
//
NPRESULT __stdcall NP_RegisterWindowHandle( HWND hWnd  )
{
	NPRESULT result = NP_ERR_DLL_NOT_FOUND;

	if( NULL != gpfNP_RegisterWindowHandle )
		result = (*gpfNP_RegisterWindowHandle)( hWnd );

	return result;
} // NP_RegisterWindowHandle()


NPRESULT __stdcall NP_UnregisterWindowHandle()
{
	NPRESULT result = NP_ERR_DLL_NOT_FOUND;

	if( NULL != gpfNP_UnregisterWindowHandle )
		result = (*gpfNP_UnregisterWindowHandle)();

	return result;
} // NP_UnregisterWindowHandle()


NPRESULT __stdcall NP_RegisterProgramProfileID( unsigned short wPPID )
{
	NPRESULT result = NP_ERR_DLL_NOT_FOUND;

	if( NULL != gpfNP_RegisterProgramProfileID )
		result = (*gpfNP_RegisterProgramProfileID)( wPPID );

	return result;
} // NP_RegisterProgramProfileID()


NPRESULT __stdcall NP_QueryVersion( unsigned short* pwVersion )
{
	NPRESULT result = NP_ERR_DLL_NOT_FOUND;

	if( NULL != gpfNP_QueryVersion )
		result = (*gpfNP_QueryVersion)( pwVersion );

	return result;
} // NP_QueryVersion()


NPRESULT __stdcall NP_RequestData( unsigned short wDataReq )
{
	NPRESULT result = NP_ERR_DLL_NOT_FOUND;

	if( NULL != gpfNP_RequestData )
		result = (*gpfNP_RequestData)( wDataReq );

	return result;
} // NP_RequestData()


NPRESULT __stdcall NP_GetData( LPTRACKIRDATA pTID )
{
	NPRESULT result = NP_ERR_DLL_NOT_FOUND;

	if( NULL != gpfNP_GetData )
		result = (*gpfNP_GetData)( pTID );

	return result;
} // NP_GetData()


NPRESULT __stdcall NP_StartCursor()
{
	NPRESULT result = NP_ERR_DLL_NOT_FOUND;

	if( NULL != gpfNP_StartCursor )
		result = (*gpfNP_StartCursor)();

	return result;
} // NP_StartCursor()


NPRESULT __stdcall NP_StopCursor()
{
	NPRESULT result = NP_ERR_DLL_NOT_FOUND;

	if( NULL != gpfNP_StopCursor )
		result = (*gpfNP_StopCursor)();

	return result;
} // NP_StopCursor()


NPRESULT __stdcall NP_StartDataTransmission()
{
	NPRESULT result = NP_ERR_DLL_NOT_FOUND;

	if( NULL != gpfNP_StartDataTransmission )
		result = (*gpfNP_StartDataTransmission)();

	return result;
} // NP_StartDataTransmission()


NPRESULT __stdcall NP_StopDataTransmission()
{
	NPRESULT result = NP_ERR_DLL_NOT_FOUND;

	if( NULL != gpfNP_StopDataTransmission )
		result = (*gpfNP_StopDataTransmission)();

	return result;
} // NP_StopDataTransmission()


//////////////////////////////////////////////////////////////////////////////
// NPClientInit() -- Loads the DLL and retrieves pointers to all exports
//
//**********************************************************************
//	Name:		NPClient_Init
//	Authors:	Retro
//	Date:		26. Feb 2003
//	Update:
//
//	Description:Made it work in C, some horrible code there I guess..
//				There´s a 200byte mem-leak here too...
//
//*********************************************************************/
NPRESULT NPClient_Init( char* csDLLPath )
{

	NPRESULT result = NP_OK;

	char csNPClientDLLFullPath[1024];

	strcpy(csNPClientDLLFullPath,csDLLPath);
	strcat(csNPClientDLLFullPath,"NPClient.dll");

	ghNPClientDLL = LoadLibrary( csNPClientDLLFullPath );
	if( NULL != ghNPClientDLL )
		{
		// Get addresses of all exported functions
		gpfNP_RegisterWindowHandle     = (PF_NP_REGISTERWINDOWHANDLE)GetProcAddress( ghNPClientDLL, "NP_RegisterWindowHandle" );
		gpfNP_UnregisterWindowHandle   = (PF_NP_UNREGISTERWINDOWHANDLE)GetProcAddress( ghNPClientDLL, "NP_UnregisterWindowHandle" );
		gpfNP_RegisterProgramProfileID = (PF_NP_REGISTERPROGRAMPROFILEID)GetProcAddress( ghNPClientDLL, "NP_RegisterProgramProfileID" );
		gpfNP_QueryVersion             = (PF_NP_QUERYVERSION)GetProcAddress( ghNPClientDLL, "NP_QueryVersion" );
		gpfNP_RequestData              = (PF_NP_REQUESTDATA)GetProcAddress( ghNPClientDLL, "NP_RequestData" );
		gpfNP_GetData                  = (PF_NP_GETDATA)GetProcAddress( ghNPClientDLL, "NP_GetData" );
		gpfNP_StartCursor              = (PF_NP_STARTCURSOR)GetProcAddress( ghNPClientDLL, "NP_StartCursor" );
		gpfNP_StopCursor               = (PF_NP_STOPCURSOR)GetProcAddress( ghNPClientDLL, "NP_StopCursor" );
		gpfNP_StartDataTransmission    = (PF_NP_STARTDATATRANSMISSION)GetProcAddress( ghNPClientDLL, "NP_StartDataTransmission" );
		gpfNP_StopDataTransmission     = (PF_NP_STOPDATATRANSMISSION)GetProcAddress( ghNPClientDLL, "NP_StopDataTransmission" );

	}
	else
		result = NP_ERR_DLL_NOT_FOUND;

	return result;

} // NPClient_Init()

//////////////////////////////////////////////////////////////////////////////

char* gcsDLLPath;

//**********************************************************************
// Function:    TrackIR_2D_Map
// Date:		26.9.2003
// Author:		Retro
//
//	Is this code legal ?
//*********************************************************************/
int TrackIR::TrackIR_2D_Map()
{

	if (!panningAllowed)
		return -1;

	TRACKIRDATA tid;

	if( NP_OK == NP_GetData( &tid ))
	{
		// xpos, ypos are in the -16383... +16383 range
		int xpos = (int)tid.fNPYaw;
		int ypos = (int)tid.fNPPitch;

		int retval = 0;

		if (xpos > Pit_2d_Yaw)				// left
		{
			retval = POV_W;
		}
		else if (xpos > -Pit_2d_Yaw)		// middle
		{
			if (ypos > Pit_2d_Pitch)		// down
				retval = POV_S;
			else if (ypos > -Pit_2d_Pitch)	// middle
				retval = -1;
			else							// up
				retval = POV_N;
		}
		else								// right
		{
			retval = POV_E;
		}

		panningAllowed = false;
	
#ifdef DEBUG_TRACKIR_STUFF
		FILE* fp = fopen("TIR_Debug.txt","at");
		fprintf(fp,"Yaw %f\t Pitch %f\t retval %i\n",tid.fNPYaw,tid.fNPPitch,retval);
		fclose(fp);
#endif
		return retval;
	}

	panningAllowed = false;

	return -1;
}

void TrackIR::Poll()
{
	TRACKIRDATA tid;

	if( NP_OK == NP_GetData( &tid ))
	{
		if (FrameSignature != tid.wPFrameSignature)
		{
			yaw = -tid.fNPYaw / 16383.f * PI;		// yaw is +-180 (PI) degrees
			pitch = tid.fNPPitch / 16383.f * PI;	// see below for limit
			roll = tid.fNPRoll / 16383.f * PI / 2;	// +-90.. bad enough imnsho
			x = tid.fNPX;
			y = tid.fNPY;
			z = tid.fNPZ;
			
			if (pitch >= PI/4)	// limit to -45 deg
			{
				pitch = PI/4;
			}
			else if (pitch <= - 0.75f * PI)	// limit to 135deg
			{
				pitch = - 0.75f*PI;
			}

			missedFrameCount = 0;
			if (isActive == false)
			{
				OTWDriver.SetHeadTracking(TRUE);	// Retro 26/09/03
				isActive = true;
			}

			FrameSignature = tid.wPFrameSignature;
		}
		else
		{
			missedFrameCount++;
			if (missedFrameCount > 100)
			{
				isActive = false;
				OTWDriver.SetHeadTracking(FALSE);	// Retro 26/09/03
			}
		}
	}
}

//**********************************************************************
// Function:    GetTrackIR_ViewValues
// Date:		26.9.2003
// Author:		Retro
//*********************************************************************/
void TrackIR::GetTrackIR_ViewValues(float* yaw, float* pitch)
{
	TRACKIRDATA tid;

	if( NP_OK == NP_GetData( &tid ))
	{
		if (FrameSignature != tid.wPFrameSignature)
		{
			*yaw = 	-tid.fNPYaw/16383.f*PI;		// yaw is +-180 (PI) degrees

			*pitch = tid.fNPPitch/16383.f*PI;	// we limit pitch to +90 (PI/2) and -45 (PI/4) degrees
			if (*pitch >= PI/4)	// limit to -45 deg
			{
				*pitch = PI/4;
			}
/*			else if (*pitch <= -PI/2)// limit to 90 deg
			{
				*pitch = -PI/2;
			}
*/			else if (*pitch <= - 0.75f*PI)	// limit to 135deg
			{
				*pitch = - 0.75f*PI;
			}

			missedFrameCount = 0;
			if ((isActive == false) && (g_bTrackIRon))
			{
				OTWDriver.SetHeadTracking(TRUE);	// Retro 26/09/03
				isActive = true;
			}

			FrameSignature = tid.wPFrameSignature;

#ifdef DEBUG_TRACKIR_STUFF
			FILE* fp = fopen("TIR_Debug.txt","at");
			fprintf(fp,"Yaw %f\t Pitch %f\n",tid.fNPYaw,tid.fNPPitch);
			fprintf(fp,"Yaw %f\t Pitch %f\n",*yaw,*pitch);
			fclose(fp);
#endif
		}
		else
		{
			missedFrameCount++;
			if (missedFrameCount > 100)
			{
				isActive = false;
				OTWDriver.SetHeadTracking(FALSE);	// Retro 26/09/03
			}
#ifdef DEBUG_TRACKIR_STUFF
			FILE* fp = fopen("TIR_Debug.txt","at");
			fprintf(fp,"Missed frame # %i, FrameSig %i, NPFrameSig %i\n",missedFrameCount,FrameSignature,tid.wPFrameSignature);
			fclose(fp);
#endif
			// yaw and pitch values stay unchanged so that looking via POV hat works
		}
	}
}

//**********************************************************************
//	Name:		GetDllLocation
//	Authors:	wk, Retro
//	Date:		26. Feb 2003
//	Update:
//
//	Description:Look in the registry for the path to the NPClient.dll..
//				Taken form the NaturalPoint sample code
//*********************************************************************/
char* GetDllLocation(char* loc)
{
	unsigned char *szValue;
	char* retval = NULL;
	DWORD dwSize;
	HKEY pKey = NULL;

	//**********************************************************************
	//open the registry key 
	//*********************************************************************/
	if (RegOpenKeyEx(	HKEY_CURRENT_USER,
						"Software\\NaturalPoint\\NATURALPOINT\\NPClient Location",
						0,
						KEY_READ,
						&pKey) != ERROR_SUCCESS)
	{
		//error condition

		return NULL;
	}

	//**********************************************************************
	//get the value from the key
	//*********************************************************************/
	if (!pKey)
		return NULL;

	//**********************************************************************
	//first discover the size of the value
	//*********************************************************************/
	if (RegQueryValueEx(pKey, "Path", NULL, NULL, NULL, &dwSize) == ERROR_SUCCESS)
	{
		//allocate memory for the buffer for the value
		szValue = (unsigned char *)malloc(dwSize);
		if (szValue != NULL)
		{
			//**********************************************************************
			//now get the value
			//*********************************************************************/
			if (RegQueryValueEx(pKey, "Path", NULL, NULL, szValue, &dwSize) == ERROR_SUCCESS)
			{
				//everything worked
//				RegCloseKey(pKey);
		 
				retval = (char*)szValue;
			}
		}	
	}
	
	RegCloseKey(pKey);
	
	return retval;
}

//**********************************************************************
// Spiffy Macro by wk that retro crippled in order to work in C
//*********************************************************************/
#define TEST_RESULT(a, b)       \
{ if(NP_OK != b)                \
	{	/*::MessageBox(0, a, "", 0);*/\
		return;                     \
	}                             \
}

//**********************************************************************
//	Name:		InitTrackIR
//	Authors:	wk, Retro
//	Date:		26. Feb 2003
//	Update:
//
//	Description:Hook up the NaturalPoint game client DLL using the wrapper module
//*********************************************************************/
void TrackIR::InitTrackIR(HWND application_window)
{
	unsigned short wNPClientVer;
	unsigned int DataFields;
	int TIRVersionMajor = -1, TIRVersionMinor = -1;	// not used anyway
	NPRESULT result;

#ifdef DEBUG_TRACKIR_STUFF
	FILE* fp = fopen("TIR_Debug.txt","at");
	fprintf(fp,"Initializing at startup...\n");
	if (g_bEnableTrackIR)
		fprintf(fp,"..with g_bEnableTrackIR ENABLED (of course.. :p)\n");
	//if (g_bTrackIR2DCockpit)
	//{
	//	fprintf(fp,"Freq %i Yaw %i Pitch %i\n",	g_nTrackIRSampleFreq/2,
	//											(int)(g_fTIR2DYawPercentage*(float)16383),
	//											(int)(g_fTIR2DPitchPercentage*(float)16383));
	//}
	fclose(fp);
#endif

	g_bEnableTrackIR = false;	// only gets set back to TRUE if init succeeds..
	g_bTrackIRon = false;

	HWND HandleGame = application_window;

	gcsDLLPath = GetDllLocation(gcsDLLPath);
	if (!gcsDLLPath)
		return;

	//**********************************************************************
	// Initialize the NPClient interface
	//*********************************************************************/
	TEST_RESULT("NPClient_Init", NPClient_Init(gcsDLLPath))

	free ( gcsDLLPath ); // uuurgh.. does this work ?

	//**********************************************************************
	// Register the app's window handle
	//*********************************************************************/
	result = NP_RegisterWindowHandle( HandleGame );
	if(result != NP_OK) // this happens if the user forgot to start the TrackIR GUI
	{ 
		// do any other error output?
//		::MessageBeep(-1);
		return;
	}
	
// 2do:	NPRESULT __stdcall 
	result = NP_RegisterProgramProfileID(1901); // Falcon ID, issued by Halstead York (NP PR Guru)

	//**********************************************************************
	// Query the NaturalPoint software version
	//*********************************************************************/
	result = NP_QueryVersion( &wNPClientVer );
	if( NP_OK == result )
	{
		TIRVersionMajor = wNPClientVer >> 8;
		TIRVersionMinor = wNPClientVer & 0x00FF;
	}

#ifdef DEBUG_TRACKIR_STUFF
	fp = fopen("TIR_Debug.txt","at");
	fprintf(fp,"Version %i.%i\n",TIRVersionMajor,TIRVersionMinor);
	fclose(fp);
#endif
	
	DataFields = NPPitch | NPYaw | NPRoll | NPX | NPY | NPZ;

	TEST_RESULT( "NP_RequestData", NP_RequestData(DataFields))
	
	TEST_RESULT("NP_StopCursor", NP_StopCursor())

	TEST_RESULT( "NP_StartDataTransmission", NP_StartDataTransmission())

	g_bEnableTrackIR = true;					// Retro 26/09/03 - init successful !
	g_bTrackIRon = true;
	OTWDriver.SetHeadTracking(TRUE);	// Retro 26/09/03

	if (PlayerOptions.Get2dTrackIR() == true)
	{
		g_nTrackIRSampleFreq = g_nTrackIRSampleFreq/2;
		Pit_2d_Yaw = (int) (g_fTIR2DYawPercentage*(float)16383);
		Pit_2d_Pitch = (int) (g_fTIR2DPitchPercentage*(float)16383);
	}

#ifdef DEBUG_TRACKIR_STUFF
	fp = fopen("TIR_Debug.txt","at");
	fprintf(fp,"Init Successful !\n");
	fclose(fp);
#endif
}

//**********************************************************************
//
// Function:    ExitTrackIR
// Date:		23.1.2003
// Author:		WK
//
// Description:		Tells TrackIR we are going...
//
//*********************************************************************/
void TrackIR::ExitTrackIR()
{
	NP_StopDataTransmission( );
	NP_StartCursor( );
	NP_UnregisterWindowHandle( );
}
