// Retro 26/09/03

//#define DEBUG_TRACKIR_STUFF

class TrackIR {
public:
	TrackIR()
			{	FrameSignature = missedFrameCount = Pit_2d_Yaw = Pit_2d_Yaw = 0;
				panningAllowed = isActive = true;
				yaw = pitch = roll = x = y = z = 0;
			};
	~TrackIR() {};

	void InitTrackIR(HWND);
	void ExitTrackIR();

	void Allow_2D_Panning()		{ panningAllowed = true; }
	bool Get_Panning_Allowed()	{ return panningAllowed; }

	void GetTrackIR_ViewValues(float*, float*);	// for 3d cockpit panning, gets yaw/pitch values in radians
	int TrackIR_2D_Map();						// for 2d cockpit panning, returns POV_N, POV_S, POV_E, POV_W values, -1 for neutral

	float getYaw() { return yaw; }
	float getPitch() { return pitch; }
	float getRoll() { return roll; }
	float getX() { return x; }
	float getY() { return y; }
	float getZ() { return z;}

	void Poll();

private:
	unsigned long FrameSignature;
	unsigned long missedFrameCount;

	bool panningAllowed;

	bool isActive;	// flag indicating if the TIR receives updates.. eg when the user hits F9 (stop TIR) then this one gets false after
					// a set time of frames (100 for now). This is used to give control back to keyboard/mouse in case tir is switched off
					// ONLY FOR THE 3d PIT THOUGHT - 2D pit just returns -1 ("don´t pan")

	int Pit_2d_Yaw;
	int Pit_2d_Pitch;

	float yaw, pitch, roll, x, y, z;
};
