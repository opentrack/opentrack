#pragma once

#define WINE_SHM_NAME "facetracknoir-wine-shm"
#define WINE_MTX_NAME "facetracknoir-wine-mtx"

struct WineSHM {
    float rx, ry, rz, tx, ty, tz;
    bool stop;
};

struct TFreeTrackData {
	int DataID;
	int CamWidth;
    int CamHeight;
    // virtual pose
    float Yaw;   // positive yaw to the left
    float Pitch; // positive pitch up
    float Roll;  // positive roll to the left
    float X;
    float Y;
    float Z;
    // raw pose with no smoothing, sensitivity, response curve etc. 
    float RawYaw;
    float RawPitch;
    float RawRoll;
    float RawX;
    float RawY;
    float RawZ;
    // raw points, sorted by Y, origin top left corner
    float X1;
    float Y1;
    float X2;
    float Y2;
    float X3;
    float Y3;
    float X4;
    float Y4;
};
typedef TFreeTrackData * PFreetrackData;

struct FTMemMap {
	TFreeTrackData data;
	HANDLE handle;
    char ProgramName[100];		// The name of the game
	char GameID[10];			// The international game-ID
	char FTNID[30];				// The FaceTrackNoIR game-ID
	char FTNVERSION[10];		// The version of FaceTrackNoIR, in which the game was first supported
};
