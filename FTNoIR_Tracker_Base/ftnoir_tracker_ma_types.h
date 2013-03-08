//
// Definitions for the Shared Memory to send the data to FaceTrackNoIR
//
static const char* MA_MM_DATA = "MA_SharedMem";
static const char* MA_FACEAPI = "MA_FaceAPI";
static const char* MA_MUTEX = "MA_Mutex";

struct TFaceData {
	int DataID;
//	smEngineHeadPoseData new_pose;
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
	FT_MA_START = 10,
	FT_MA_STOP  = 20,
	FT_MA_SHOW_CAM = 30,
	FT_MA_SET_PAR_FILTER = 50,
	FT_MA_EXIT  = 100
};
