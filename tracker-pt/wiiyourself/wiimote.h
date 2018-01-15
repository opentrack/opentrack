// _______________________________________________________________________________
//
//	 - WiiYourself! - native C++ Wiimote library  v1.15
//	  (c) gl.tter 2007-10 - http://gl.tter.org
//
//	  see License.txt for conditions of use.  see History.txt for change log.
// _______________________________________________________________________________
//
//  wiimote.h  (tab = 4 spaces)

#ifdef _MSC_VER // VC
# pragma once
#endif

#ifndef _WIIMOTE_H
# define _WIIMOTE_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>		// auto Unicode/Ansi support
#include <queue>		// for HID write method
#include <list>			// for state recording
 
#ifndef QWORD
 typedef unsigned __int64 QWORD;
#endif

#ifdef _MSC_VER			   // VC-specific: _DEBUG build only _ASSERT() sanity checks
# include <crtdbg.h>
#elif defined(__MINGW32__) // define NDEBUG to disable assert
# include <assert.h>
# define _ASSERT assert
#else
# define _ASSERT(x) ((void)0) // (add your compiler's implementation if you like)
#endif

#ifdef SWIGWRAPPER		// Python Wrapper
#include "Python/wiimote_state.i"
#else
#include "wiimote_state.h"
#endif

// configs:
//#define USE_DYNAMIC_HIDQUEUE // deprecated

//  we request periodic status report updates to refresh the battery level
//   and to detect connection loss (through failed writes)
#define REQUEST_STATUS_EVERY_MS		1000
#define DETECT_MPLUS_EVERY_MS		1000
#define DETECT_MPLUS_COUNT			1	 // # of tries in quick succession

//  all threads (read/parse, audio streaming, async rumble...) use this priority
#define WORKER_THREAD_PRIORITY		THREAD_PRIORITY_HIGHEST

 // internals
#define	WIIYOURSELF_VERSION_MAJOR	1
#define	WIIYOURSELF_VERSION_MINOR1	1
#define	WIIYOURSELF_VERSION_MINOR2	5
//#define WIIYOURSELF_VERSION_BETA	// not defined for non-beta releases
#define WIIYOURSELF_VERSION_STR		_T("1.15")

// array sizes
#define	TOTAL_BUTTON_BITS	16	// Number of bits for (Classic)ButtonNameFromBit[]
#define	TOTAL_FREQUENCIES	10	// Number of frequencies (see speaker_freq[])

 // clarity
typedef HANDLE EVENT;


// state data changes can be signalled to the app via a callback.  Set the wiimote
//  object's 'ChangedCallback' any time to enable them, or alternatively inherit
//   from the wiimote object and override the ChangedNotifier() virtual method.

//  of flags indicating which state has changed since the last callback.
typedef void (*state_changed_callback)	(class wiimote	     &owner,
										 state_change_flags  changed,
										 const wiimote_state &new_state);

// internals
typedef BOOLEAN (__stdcall *hidwrite_ptr)(HANDLE HidDeviceObject,
										  PVOID  ReportBuffer,
										  ULONG  ReportBufferLength);

// (global due to Python wrapper)
struct wiimote_state_event {
	DWORD		  time_ms;	// system timestamp in milliseconds
	wiimote_state state;
	};

// wiimote class - connects and manages a wiimote and its optional extensions
//                 (Nunchuk/Classic Controller), and exposes their state
class wiimote : public wiimote_state
	{
	public:
		wiimote ();
		virtual ~wiimote ();

	public:
		// these can be used to identify Connect()ed wiimote objects (if both
		//  are unconnected they will pass the compare as their handles are invalid)
		inline bool operator == (const wiimote& remote)
			{ return Handle == remote.Handle; }
		inline bool operator != (const wiimote& remote)
			{ return Handle != remote.Handle; }

		// wiimote data input mode (use with SetReportType())
		//  (only enable what you need to save battery power)
		enum input_report
			{
			// combinations if buttons/acceleration/IR/Extension data
			IN_BUTTONS				 = 0x30,
			IN_BUTTONS_ACCEL		 = 0x31,
			IN_BUTTONS_ACCEL_IR		 = 0x33, // reports IR EXTENDED data (dot sizes)
			IN_BUTTONS_ACCEL_EXT	 = 0x35,
			IN_BUTTONS_ACCEL_IR_EXT	 = 0x37, // reports IR BASIC data (no dot sizes)
			IN_BUTTONS_BALANCE_BOARD = 0x32, // must use this for the balance board
			};
		// string versions 
		static const TCHAR* ReportTypeName [];


	public: // convenience accessors:
		inline bool	IsConnected			  () const { return bStatusReceived; }
		// if IsConnected() unexpectedly returns false, connection was probably lost
		inline bool	ConnectionLost		  () const { return bConnectionLost; }
		inline bool IsBalanceBoard		  () const { return (Internal.bExtension &&
							(Internal.ExtensionType==wiimote_state::BALANCE_BOARD)); }
		inline bool	NunchukConnected	  () const { return (Internal.bExtension &&
							(Internal.ExtensionType==wiimote_state::NUNCHUK)); }
		inline bool	ClassicConnected	  () const { return (Internal.bExtension &&
							(Internal.ExtensionType==wiimote_state::CLASSIC)); }
		inline bool MotionPlusConnected   () const { return bMotionPlusDetected; }
		inline bool MotionPlusEnabled	  () const { return bMotionPlusEnabled; }
		inline bool MotionPlusHasExtension() const { return bMotionPlusExtension; }
		inline bool	IsPlayingAudio		  () const { return (Internal.Speaker.Freq &&
															 Internal.Speaker.Volume); }
		inline bool	IsPlayingSample		  () const { return IsPlayingAudio() &&
															(CurrentSample != NULL); }
		inline bool	IsUsingHIDwrites	  () const { return bUseHIDwrite; }
		inline bool	IsRecordingState	  () const { return Recording.bEnabled; }

		static inline unsigned TotalConnected() { return _TotalConnected; }


	public: // data
		QWORD UniqueID;		   // constructed from device-specific calibration info.
							   //  Note this is not guaranteed to be truly unique
							   //  as several devices may contain the same calibration
							   //  vluaes - but unique amongst a small number of 
							   //  devices.
#ifdef ID2_FROM_DEVICEPATH
		QWORD UniqueID2;	   // (low-reliabilty, left for reference)
							   //  constructed from the 'device path' string (as
							   //  reported by the OS/stack).  This is hopefully
							   //  unique as long as the devices remain installed
							   //  (or at least paired).
#endif
		// optional callbacks - set these to your own fuctions (if required)
		state_changed_callback  ChangedCallback;
		//  you can avoid unnecessary callback overhead by specifying a mask
		//   of which state changes should trigger them (default is any)
		state_change_flags		CallbackTriggerFlags;
		// alternatively, inherit from this class and override this virtual function:
		virtual void			ChangedNotifier (state_change_flags  changed,
												 const wiimote_state &new_state) {};

		// get the button name from its bit index (some bits are unused)
		static const TCHAR*		   ButtonNameFromBit [TOTAL_BUTTON_BITS];
		static const TCHAR*		GetButtonNameFromBit (unsigned index)
			{
			_ASSERT(index < TOTAL_BUTTON_BITS);
			if(index >= TOTAL_BUTTON_BITS)
				return _T("[invalid index]");
			return ButtonNameFromBit[index];
			}

		// same for the Classic Controller
		static const TCHAR*		   ClassicButtonNameFromBit [TOTAL_BUTTON_BITS];
		static const TCHAR*		GetClassicButtonNameFromBit (unsigned index)
			{
			_ASSERT(index < TOTAL_BUTTON_BITS);
			if(index >= TOTAL_BUTTON_BITS)
				return _T("[invalid index]");
			return ClassicButtonNameFromBit[index];
			}

		// get the frequency from speaker_freq enum
		static const unsigned	   FreqLookup [TOTAL_FREQUENCIES];
		static const unsigned	GetFreqLookup (unsigned index)
			{
			_ASSERT(index < TOTAL_FREQUENCIES);
			if(index >= TOTAL_FREQUENCIES)
				return 0;
			return FreqLookup[index];
			}

	public: // methods
		// call Connect() first - returns true if wiimote was found & enabled
		//  - 'wiimote_index' specifies which *installed* (not necessarily
		//     *connected*) wiimote should be tried (1 = first, 2 = 2nd etc).
		//    if you just want the first *available* wiimote that isn't already
		//     in use, pass in FIRST_AVAILABLE (default).
		//  - 'force_hidwrites' forces HID output method (it's auto-selected
		//     when needed and less efficient, so only force for testing).
		static const unsigned FIRST_AVAILABLE = 0xffffffff;
		bool Connect				 (unsigned wiimote_index = FIRST_AVAILABLE,
									  bool force_hidwrites   = false);
		// disconnect from the controller and stop reading data from it
		void Disconnect				 ();
		// set wiimote reporting mode (call after Connnect())
		//  continous = true forces the wiimote to send constant updates, even when
		//			     nothing has changed.
		//			  = false only sends data when something has changed (note that
		//			     acceleration data will cause frequent updates anyway as it
		//			     jitters even when the wiimote is stationary)
		void SetReportType			 (input_report type, bool continuous = false);
		
		// toggle the MotionPlus extension.  Call MotionPlusDetected() first to
		//  see if it's attached.  Unlike normal extensions, the MotionPlus does
		//  not report itself as one until enabled.  Once done, it then replaces
		//  any standard extension attached to it, so be sure to disable it
		//  if you want to read those (it's not currently known of both can
		//  be read simultaneously).
		bool EnableMotionPlus  ();
		bool DisableMotionPlus ();
	
		// this is used to remove unwanted 'at rest' offsets, currently only from
		//  the Balance Board.  make sure there is no weight on the board before
		//  calling this. it reads the current sensor values and then removes them
		//  offsets from all subsequent KG and LB state values (the 'raw' values
		//  are never modified).
		void CalibrateAtRest		 ();
		// NOTE: the library automatically calls this when the first weight values
		//        come in after Connect()ion, but if the device wasn't at rest at
		//        the time the app can call it again later.

		// to read the state via polling (reading the public state data direct from
		//  the wiimote object) you must call RefreshState() at the top of every pass.
		//  returns a combination of flags to indicate which state (if any) has
		//  changed since the last call.
		state_change_flags RefreshState ();

		// reset the wiimote (changes report type to non-continuous buttons-only,
		//					  clears LEDs & rumble, mutes & disables speaker)
		void Reset					 ();
		// set/clear the wiimote LEDs
		void SetLEDs				 (BYTE led_bits); // bits 0-3 are valid
		// set/clear rumble
		void SetRumble				 (bool on);
		// alternative - rumble for a fixed amount of time (asynchronous)
		void RumbleForAsync			 (unsigned milliseconds);

		// *experimental* speaker support:
		bool MuteSpeaker			 (bool on);
		bool EnableSpeaker			 (bool on);
		bool PlaySquareWave			 (speaker_freq freq, BYTE volume = 0x40);
		//  note: PlaySample currently streams from the passed-in wiimote_sample -
		//		   don't delete it until playback has stopped.
		bool PlaySample				 (const		   wiimote_sample &sample,
									  BYTE		   volume		 = 0x40,
									  speaker_freq freq_override = FREQ_NONE);

		//  16bit mono sample loading/conversion to native format:
		//   .wav sample
		static bool Load16bitMonoSampleWAV	 (const TCHAR*   filepath,
											  wiimote_sample &out);
		//   raw 16bit mono audio data (can be signed or unsigned)
		static bool Load16BitMonoSampleRAW	 (const TCHAR*   filepath,
											  bool		     _signed,
											  speaker_freq   freq,
											  wiimote_sample &out);
		//   converts a 16bit mono sample array to a wiimote_sample
		static bool Convert16bitMonoSamples  (const short*   samples,
											  bool		     _signed,
											  DWORD		     length,
											  speaker_freq   freq,
											  wiimote_sample &out);
		
		// state recording - records state snapshots to a 'state_history' supplied
		//					  by the caller.  states are timestamped and only added
		//					  to the list when the specified state changes.
#ifndef SWIG // !Python Wrapper
		typedef wiimote_state_event state_event;
#endif
		typedef std::list<state_event> state_history;
		static const unsigned UNTIL_STOP = 0xffffffff;
		// - pass in a 'state_history' list, and don't destroy/change it until
		//	  recording is stopped.  note the list will be cleared first.
		// - you can request a specific duration (and use IsRecordingState() to detect
		//    the end), or UNTIL_STOP.  StopRecording() can be called either way.
		// - you can use 'change trigger' to specify specific state changes that will
		//    trigger an insert into the history (others are then ignored).
		void RecordState  (state_history	  &events_out,
						   unsigned			  max_time_ms	 = UNTIL_STOP,
						   state_change_flags change_trigger = CHANGED_ALL);
		void StopRecording ();


	private: // methods
		// start reading asynchronously from the controller
		bool BeginAsyncRead			();
		// request status update (battery level, extension status etc)
		void RequestStatusReport	();
		// read address or register from Wiimote asynchronously (the result is
		//  parsed internally whenever it arrives)
		bool ReadAddress			(int address, short size);
		// write a single BYTE to a wiimote address or register
		inline void WriteData		(int address, BYTE data) { WriteData(address, 1, &data); }
		// write a BYTE array to a wiimote address or register
		void WriteData				(int address, BYTE size, const BYTE* buff);
		// callback when data is ready to be processed
		void OnReadData				(DWORD bytes_read);
		// parse individual reports by type
		int	 ParseInput				(BYTE* buff);
		// detects if MotionPlus is attached (it doesn't report as a normal
		//  extesnion until it is enabled)
		void DetectMotionPlusExtensionAsync ();
		// initializes an extension when plugged in.
		void InitializeExtension	();
		// parses a status report
		int	 ParseStatus		    (BYTE* buff);
		// parses the buttons
		int	 ParseButtons			(BYTE* buff);
		// parses accelerometer data
		int	 ParseAccel				(BYTE* buff);
		bool EstimateOrientationFrom(wiimote_state::acceleration &accel);
		void ApplyJoystickDeadZones (wiimote_state::joystick &joy);
		// parses IR data from report
		int  ParseIR				(BYTE* buff);
		// parses data from an extension.
		int  ParseExtension			(BYTE* buff, unsigned offset);
		// parses data returned from a read report
		int  ParseReadAddress		(BYTE* buff);
		// reads calibration information stored on Wiimote
		void ReadCalibration		();
		float GetBalanceValue(short sensor, short min, short mid, short max);
		// turns on the IR sensor (the mode must match the reporting mode caps)
		void EnableIR				(wiimote_state::ir::mode mode);
		// disables the IR sensor
		void DisableIR				();
		// writes a report to the Wiimote (NULL = use 'WriteBuff')
		bool WriteReport			(BYTE* buff);
		bool StartSampleThread		();
		// returns the rumble BYTE that needs to be sent with reports.
		inline BYTE  GetRumbleBit	()	const { return Internal.bRumble? 0x01 : 0x00; }

		// static thread funcs:
		static unsigned __stdcall ReadParseThreadfunc   (void* param);
		static unsigned __stdcall AsyncRumbleThreadfunc (void* param);
		static unsigned __stdcall SampleStreamThreadfunc(void* param);
		static unsigned __stdcall HIDwriteThreadfunc	(void* param);

	private: // data
		// wiimote output comands
		enum output_report
			{
			OUT_NONE			= 0x00,
			OUT_LEDs			= 0x11,
			OUT_TYPE			= 0x12,
			OUT_IR				= 0x13,
			OUT_SPEAKER_ENABLE	= 0x14,
			OUT_STATUS			= 0x15,
			OUT_WRITEMEMORY		= 0x16,
			OUT_READMEMORY		= 0x17,
			OUT_SPEAKER_DATA	= 0x18,
			OUT_SPEAKER_MUTE	= 0x19,
			OUT_IR2				= 0x1a,
			};
		// input reports used only internally:
		static const int IN_STATUS						= 0x20;
		static const int IN_READADDRESS					= 0x21;
		// wiimote device IDs:
		static const int VID							= 0x057e; // 'Nintendo'
		static const int PID							= 0x0306; // 'Wiimote'
		// we could find this out the hard way using HID, but it's 22
		static const int REPORT_LENGTH					= 22;
		// wiimote registers
		static const int REGISTER_CALIBRATION			= 0x0016;
		static const int REGISTER_IR					= 0x4b00030;
		static const int REGISTER_IR_SENSITIVITY_1		= 0x4b00000;
		static const int REGISTER_IR_SENSITIVITY_2		= 0x4b0001a;
		static const int REGISTER_IR_MODE				= 0x4b00033;
		static const int REGISTER_EXTENSION_INIT1		= 0x4a400f0;
		static const int REGISTER_EXTENSION_INIT2		= 0x4a400fb;
		static const int REGISTER_EXTENSION_TYPE		= 0x4a400fa;
		static const int REGISTER_EXTENSION_CALIBRATION	= 0x4a40020;
		static const int REGISTER_BALANCE_CALIBRATION	= 0x4a40024;
		static const int REGISTER_MOTIONPLUS_DETECT		= 0x4a600fa;
		static const int REGISTER_MOTIONPLUS_INIT		= 0x4a600f0;
		static const int REGISTER_MOTIONPLUS_ENABLE		= 0x4a600fe;

		HANDLE			 Handle;		  // read/write device handle
		OVERLAPPED		 Overlapped;	  // for async Read/WriteFile() IO
		HANDLE			 ReadParseThread; // waits for overlapped reads & parses result
		EVENT			 DataRead;		  // signals overlapped read complete
		bool			 bUseHIDwrite;	  // alternative write method (less efficient
										  //  but required for some BT stacks (eg. MS')
		// HidD_SetOutputReport is only supported from XP onwards, so detect &
		//  load it dynamically:
		static HMODULE	 HidDLL;	
		static hidwrite_ptr _HidD_SetOutputReport;
		
		volatile bool	 bStatusReceived;	  // for output method detection
		volatile bool	 bConnectInProgress;  // don't handle extensions until complete
		volatile bool	 bInitInProgress;	  // stop regular requests until complete
		volatile bool	 bEnablingMotionPlus; // for special init codepath
		volatile bool	 bConnectionLost;	  // auto-Disconnect()s if set
volatile int	 MotionPlusDetectCount;		  // waiting for the result
		volatile bool	 bMotionPlusDetected;
		volatile bool	 bMotionPlusEnabled;
		volatile bool	 bMotionPlusExtension;// detected one plugged into MotionPlus
		volatile bool	 bCalibrateAtRest;	  // as soon as the first sensor values 											  //  come in after a Connect() call.
		static unsigned	 _TotalCreated;
		static unsigned	 _TotalConnected;
		input_report	 ReportType;	      // type of data the wiimote delivers	
		// read buffer
		BYTE			 ReadBuff  [REPORT_LENGTH];
		// for polling: state is updated on a thread internally, and made only
		//  made public via RefreshState()
		CRITICAL_SECTION StateLock;	
		wiimote_state	 Internal;
		state_change_flags InternalChanged;   // state changes since last RefreshState()
		// periodic status report requests (for battery level and connection loss
		//  detection)
		DWORD			  NextStatusTime;
		DWORD			  NextMPlusDetectTime;// gap between motion plus detections
		DWORD			  MPlusDetectCount;	  // # of detection tries in quick succesion
		// async Hidd_WriteReport() thread
		HANDLE			  HIDwriteThread;
#ifdef USE_DYNAMIC_HIDQUEUE
		std::queue<BYTE*> HIDwriteQueue;
#else
		// fixed-size queue (to eliminate glitches caused by frequent dynamic memory
		//	allocations)
		struct hid
			{
			hid () : Queue(NULL), ReadIndex(0), WriteIndex(0) {}

			// Increase the static queue size if you get ASSERTs signalling an
			//  overflow (too many reports queued up before being sent by the write
			//  thread).  These asserts are harmless though if caused as a result of
			//  loosing the wiimote connection (eg. battery runs out, or wiimote is
			//  unpaired by holding the power button).
			// Note: MAX_QUEUE_ENTRIES _must_ be a power-of-2, as it
			//  uses index wraparound optimisations.
			static const unsigned MAX_QUEUE_ENTRIES = 1<<7;
		
			inline bool IsEmpty() const { return (ReadIndex == WriteIndex); }

			bool Allocate	()	{ // allocate memory (only when needed)
								_ASSERT(!Queue); if(Queue) return true;
								ReadIndex = WriteIndex = 0;
								Queue = new queue_entry[MAX_QUEUE_ENTRIES];
								_ASSERT(Queue); return (Queue != NULL);
								}
			void Deallocate ()	{
								if(!Queue) return;
								delete[] Queue; Queue = NULL;
								ReadIndex = WriteIndex = 0;
								}
								
			struct queue_entry
				{
				queue_entry() { memset(Report, 0, sizeof(Report)); }
				
				BYTE Report [REPORT_LENGTH];
				} *Queue;
			
			unsigned ReadIndex, WriteIndex;
			} HID;
#endif
		CRITICAL_SECTION  HIDwriteQueueLock;  // queue must be locked before being  modified

		// async rumble
		HANDLE			 AsyncRumbleThread;	  // automatically disables rumble if requested
		volatile DWORD	 AsyncRumbleTimeout;
		// orientation estimation
		unsigned		 WiimoteNearGUpdates;
		unsigned		 NunchukNearGUpdates;
		// audio
		HANDLE			 SampleThread;
		const wiimote_sample* volatile CurrentSample;	// otherwise playing square wave
		// state recording
		struct recording
			{
			volatile bool	bEnabled;
			state_history	*StateHistory;
			volatile DWORD	StartTimeMS;
			volatile DWORD	EndTimeMS;		// can be UNTIL_STOP
			unsigned		TriggerFlags;	// wiimote changes trigger a state event
			unsigned		ExtTriggerFlags;// extension changes "
			} Recording;
	};

#endif // _WIIMOTE_H