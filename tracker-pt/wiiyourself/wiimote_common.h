// _______________________________________________________________________________
//
//	 - WiiYourself! - native C++ Wiimote library  v1.15 RC
//	  (c) gl.tter 2007-9 - http://gl.tter.org
//
//	  see License.txt for conditions of use.  see History.txt for change log.
// _______________________________________________________________________________
//
//  wiimote_common.h  (tab = 4 spaces)

// speaker support:
enum speaker_freq
	{
	// (keep in sync with FreqLookup in wiimote.cpp)
	FREQ_NONE	= 0,
	// my PC can't keep up with these using bUseHIDwrite, so I haven't
	//  been able to tune them yet
	FREQ_4200HZ = 1,
	FREQ_3920HZ = 2,
	FREQ_3640HZ = 3,
	FREQ_3360HZ = 4,
	// these were tuned until the square-wave was glitch-free on my remote -
	//  may not be exactly right
	FREQ_3130HZ = 5,	// +190
	FREQ_2940HZ = 6,	// +180
	FREQ_2760HZ = 7,	// +150
	FREQ_2610HZ = 8,	// +140
	FREQ_2470HZ = 9,
	};

// wiimote_sample - holds the audio sample in the native wiimote format
struct wiimote_sample
	{
	wiimote_sample() : samples(NULL), length(0), freq(FREQ_NONE) {}
	BYTE*		 samples;
	DWORD		 length;
	speaker_freq freq;
	};

// flags & masks that indicate which part(s) of the wiimote state have changed
enum state_change_flags
	{
	// state didn't change at all
	NO_CHANGE					   = 0,

	// Wiimote specific:
	CONNECTED					   = 1<<0, // wiimote just connected
	CONNECTION_LOST				   = 1<<1,
	BATTERY_CHANGED				   = 1<<2,
	BATTERY_DRAINED				   = 1<<3, // close to empty
	LEDS_CHANGED				   = 1<<4, // (probably redudant as wiimmote never
	BUTTONS_CHANGED				   = 1<<5, //  changes them unless requested)
	ACCEL_CHANGED				   = 1<<6,
	ORIENTATION_CHANGED			   = 1<<7,
	IR_CHANGED					   = 1<<8,
	//  all wiimote flags
	WIIMOTE_CHANGED				   = CONNECTION_LOST|BATTERY_CHANGED|BATTERY_DRAINED|
								     LEDS_CHANGED|BUTTONS_CHANGED|ACCEL_CHANGED|
								     ORIENTATION_CHANGED|IR_CHANGED,
	// - Extensions -:
	//  Nunchuk:
	NUNCHUK_CONNECTED			   = 1<<9,
	NUNCHUK_BUTTONS_CHANGED		   = 1<<10,
	NUNCHUK_ACCEL_CHANGED		   = 1<<11,
	NUNCHUK_ORIENTATION_CHANGED	   = 1<<12,
	NUNCHUK_JOYSTICK_CHANGED	   = 1<<13,
	//   all flags
	NUNCHUK_CHANGED				   = NUNCHUK_CONNECTED|NUNCHUK_BUTTONS_CHANGED|
								     NUNCHUK_ACCEL_CHANGED|NUNCHUK_ORIENTATION_CHANGED|
								     NUNCHUK_JOYSTICK_CHANGED,
	//  Classic Controller (inc. Guitars etc):
	CLASSIC_CONNECTED			   = 1<<14,
	CLASSIC_BUTTONS_CHANGED		   = 1<<15,
	CLASSIC_JOYSTICK_L_CHANGED	   = 1<<16,
	CLASSIC_JOYSTICK_R_CHANGED	   = 1<<17,
	CLASSIC_TRIGGERS_CHANGED	   = 1<<18,
	//   all flags
	CLASSIC_CHANGED				   = CLASSIC_CONNECTED|CLASSIC_BUTTONS_CHANGED|
								     CLASSIC_JOYSTICK_L_CHANGED|
								     CLASSIC_JOYSTICK_R_CHANGED|
									 CLASSIC_TRIGGERS_CHANGED,
	//  Balance Board:
	BALANCE_CONNECTED			   = 1<<19,
	BALANCE_WEIGHT_CHANGED		   = 1<<20,
	//   all flags
	BALANCE_CHANGED				   = BALANCE_CONNECTED|BALANCE_WEIGHT_CHANGED,

	//  Motion Plus
	MOTIONPLUS_DETECTED			   = 1<<21, // attached but not enabled
	MOTIONPLUS_ENABLED			   = 1<<22,
	MOTIONPLUS_SPEED_CHANGED	   = 1<<23,
	MOTIONPLUS_EXTENSION_CONNECTED = 1<<24,		// an extension is found in the
												//  MotionPlus port
	MOTIONPLUS_EXTENSION_DISCONNECTED = 1<<25, // it was disconnected
	//   all flags
	MOTIONPLUS_CHANGED			   = MOTIONPLUS_DETECTED|MOTIONPLUS_ENABLED|
									 MOTIONPLUS_SPEED_CHANGED|
									 MOTIONPLUS_EXTENSION_CONNECTED|
									 MOTIONPLUS_EXTENSION_DISCONNECTED,
	//  General:
	EXTENSION_DISCONNECTED		 = 1<<26,
	EXTENSION_PARTIALLY_INSERTED = 1<<27,
	EXTENSION_CONNECTED			 = NUNCHUK_CONNECTED|CLASSIC_CONNECTED|
								   BALANCE_CONNECTED|MOTIONPLUS_ENABLED,
	EXTENSION_CHANGED			 = EXTENSION_DISCONNECTED|NUNCHUK_CHANGED|
								   CLASSIC_CHANGED|BALANCE_CHANGED|MOTIONPLUS_CHANGED,
	//  ALL flags:
	CHANGED_ALL					 = WIIMOTE_CHANGED|EXTENSION_CHANGED,
	};
