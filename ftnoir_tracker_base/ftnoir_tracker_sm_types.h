//
// Definitions for the Shared Memory to send the data to FaceTrackNoIR
//
#define SM_MM_DATA "SM_SharedMem"
#define SM_FACEAPI "SM_FaceAPI"
#define SM_MUTEX "SM_Mutex"

#include "faceapi/stdafx.h"
#include <sm_api.h>

struct TFaceData {
	int DataID;
	smEngineHeadPoseData new_pose;
};
typedef TFaceData * PFaceData;

struct SMMemMap {
	int command;					// Command from FaceTrackNoIR
	int status;						// Status from faceAPI
	TFaceData data;
	HANDLE handle;
	int state;
	int par_val_int;				// Value of parameter, indicated by 'command'
	int par_val_float;
	int initial_filter_level;		// Internal faceAPI Filter level
	int handshake;
};
typedef SMMemMap * PSMMemMap;

enum FTNoIR_Tracker_Command {
	FT_SM_START = 10,
	FT_SM_STOP  = 20,
	FT_SM_SHOW_CAM = 30,
	FT_SM_SET_PAR_FILTER = 50,
	FT_SM_EXIT  = 100
};
