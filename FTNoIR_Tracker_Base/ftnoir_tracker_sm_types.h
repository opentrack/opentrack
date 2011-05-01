//
// Definitions for the Shared Memory to send the data to FaceTrackNoIR
//
static const char* SM_MM_DATA = "SM_SharedMem";
static const char* SM_FACEAPI = "SM_FaceAPI";
static const char* SM_MUTEX = "SM_Mutex";

#include "sm_api.h"

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
};
typedef SMMemMap * PSMMemMap;

enum FTNoIR_Tracker_Command {
	FT_SM_START = 10,
	FT_SM_STOP  = 20,
	FT_SM_SHOW_CAM = 30,
	FT_SM_EXIT  = 100
};
