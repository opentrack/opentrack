// _______________________________________________________________________________
//
//	 - WiiYourself! - native C++ Wiimote library  v1.15
//	  (c) gl.tter 2007-10 - http://gl.tter.org
//
//	  see License.txt for conditions of use.  see History.txt for change log.
// _______________________________________________________________________________
//
//  wiimote.cpp  (tab = 4 spaces)

#include "wiimote.h"
#include <cmath>
#include <algorithm>    // std::min
#include <iterator>     // std::size
#include <tchar.h>
#include <setupapi.h>
extern "C" {
#include <hidsdi.h>
}
#include <sys/types.h>	// for _stat
#include <sys/stat.h>	// "
#include <process.h>	// for _beginthreadex()
#include <mmreg.h>		// for WAVEFORMATEXTENSIBLE
#include <mmsystem.h>	// for timeGetTime()

// ------------------------------------------------------------------------------------
// helpers
// ------------------------------------------------------------------------------------
template<class T> inline T sign  (const T& val)  { return (val<0)? T(-1) : T(1); }
template<class T> inline T square(const T& val)  { return val*val; }

// ------------------------------------------------------------------------------------
//  Tracing & Debugging
// ------------------------------------------------------------------------------------

static_assert(sizeof(TCHAR) == sizeof(char));

template<typename... xs>
[[maybe_unused]]
static void trace_ (const char* fmt, const xs&... args)
{
    fprintf(stderr, fmt, args...);
    fprintf(stderr, "\n");
    fflush(stderr);
}

template<typename... xs>
[[maybe_unused]]
static inline void disabled_trace_(const char*, const xs&...) {}

#define TRACE trace_
#define WARN  trace_

#ifndef TRACE
# define TRACE disabled_trace_
#endif
#ifndef DEEP_TRACE
# define DEEP_TRACE disabled_trace_
#endif
#ifndef WARN
# define WARN disabled_trace_
#endif

// uncomment any of these for deeper debugging:
//#define BEEP_DEBUG_READS
//#define BEEP_DEBUG_WRITES
//#define BEEP_ON_ORIENTATION_ESTIMATE
//#define BEEP_ON_PERIODIC_STATUSREFRESH

// ------------------------------------------------------------------------------------
//  wiimote
// ------------------------------------------------------------------------------------
// class statics
unsigned	   wiimote::_TotalConnected		  = 0;

// (keep in sync with 'speaker_freq'):
const unsigned wiimote::FreqLookup [TOTAL_FREQUENCIES] = 
								{    0, 4200, 3920, 3640, 3360,
								  3130,	2940, 2760, 2610, 2470 };

const TCHAR* const wiimote::ButtonNameFromBit		 [TOTAL_BUTTON_BITS] =
								{ _T("Left") , _T("Right"), _T("Down"), _T("Up"),
								  _T("Plus") , _T("??")   , _T("??")  , _T("??") ,
								  _T("Two")  , _T("One")  , _T("B")   , _T("A") ,
								  _T("Minus"), _T("??")   , _T("??")  , _T("Home") };

const TCHAR* const wiimote::ClassicButtonNameFromBit [TOTAL_BUTTON_BITS] =
								{ _T("??")   , _T("TrigR")  , _T("Plus") , _T("Home"),
								  _T("Minus"), _T("TrigL") , _T("Down") , _T("Right") ,
								  _T("Up")   , _T("Left")   , _T("ZR")   , _T("X") ,
								  _T("A")    , _T("Y")      , _T("B")    , _T("ZL") };
// ------------------------------------------------------------------------------------
wiimote::wiimote ()
	:
	DataRead			 (CreateEvent(NULL, FALSE, FALSE, NULL)),
	Handle				 (INVALID_HANDLE_VALUE),
	ReportType			 (IN_BUTTONS),
	bStatusReceived		 (false), // for output method detection
	bConnectInProgress	 (true ),
	bInitInProgress		 (false),
	bEnablingMotionPlus	 (false),
	bConnectionLost		 (false), // set if write fails after connection
	bMotionPlusDetected	 (false),
	bMotionPlusEnabled	 (false),
	bMotionPlusExtension (false),
	bCalibrateAtRest	 (false),
	bUseHIDwrite		 (false), // if OS supports it
	ChangedCallback		 (NULL),
	CallbackTriggerFlags (CHANGED_ALL),
	InternalChanged		 (NO_CHANGE),
	CurrentSample		 (NULL),
	HIDwriteThread		 (NULL),
	ReadParseThread		 (NULL),
	SampleThread		 (NULL),
	AsyncRumbleThread	 (NULL),
	AsyncRumbleTimeout	 (0),
	UniqueID			 (0)	// not _guaranteed_ unique, see comments in header
#ifdef ID2_FROM_DEVICEPATH		// (see comments in header)
	// UniqueID2			 (0)	
#endif
	{
	_ASSERT(DataRead != INVALID_HANDLE_VALUE);

	// clear our public and private state data completely (including deadzones)
	Clear		  (true);
	Internal.Clear(true);

	// and the state recording vars
	memset(&Recording, 0, sizeof(Recording));

	// for overlapped IO (Read/WriteFile)
	memset(&Overlapped, 0, sizeof(Overlapped));
	Overlapped.hEvent	  = DataRead;
	Overlapped.Offset	  =
	Overlapped.OffsetHigh = 0;

	// for async HID output method
	InitializeCriticalSection(&HIDwriteQueueLock);
	// for polling
	InitializeCriticalSection(&StateLock);

	// request millisecond timer accuracy
	timeBeginPeriod(1);		
	}
// ------------------------------------------------------------------------------------
wiimote::~wiimote ()
{
	Disconnect();

	// events & critical sections are kept open for the lifetime of the object,
	//  so tidy them up here:
	if(DataRead != INVALID_HANDLE_VALUE)
		CloseHandle(DataRead);

	DeleteCriticalSection(&HIDwriteQueueLock);
	DeleteCriticalSection(&StateLock);

	// tidy up timer accuracy request
	timeEndPeriod(1);
}

// ------------------------------------------------------------------------------------
bool wiimote::Connect (unsigned wiimote_index, bool force_hidwrites)
	{
	if(wiimote_index == FIRST_AVAILABLE)
		TRACE(_T("Connecting first available Wiimote:"));
	else
		TRACE(_T("Connecting Wiimote %u:"), wiimote_index);

	// auto-disconnect if user is being naughty
	if(IsConnected())
		Disconnect();

	// get the GUID of the HID class
	GUID guid;
	HidD_GetHidGuid(&guid);

	// get a handle to all devices that are part of the HID class
	// Brian: Fun fact:  DIGCF_PRESENT worked on my machine just fine.  I reinstalled
	//   Vista, and now it no longer finds the Wiimote with that parameter enabled...
	HDEVINFO dev_info = SetupDiGetClassDevs(&guid, NULL, NULL, DIGCF_DEVICEINTERFACE);// | DIGCF_PRESENT);
	if(!dev_info) {
		WARN(_T("couldn't get device info"));
		return false;
		}

	// enumerate the devices
	SP_DEVICE_INTERFACE_DATA didata;
	didata.cbSize = sizeof(didata);
	
	unsigned index			= 0;
	unsigned wiimotes_found = 0;
	while(SetupDiEnumDeviceInterfaces(dev_info, NULL, &guid, index, &didata))
		{
		// get the buffer size for this device detail instance
		DWORD req_size = 0;
		(void)SetupDiGetDeviceInterfaceDetail(dev_info, &didata, NULL, 0, &req_size, NULL);
		if (req_size < sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA))
			WARN(_T("couldn't get device size for %u"), index);

		// (bizarre way of doing it) create a buffer large enough to hold the
		//  fixed-size detail struct components, and the variable string size
		SP_DEVICE_INTERFACE_DETAIL_DATA *didetail =
								(SP_DEVICE_INTERFACE_DETAIL_DATA*) new BYTE[req_size];
		_ASSERT(didetail);
		didetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// now actually get the detail struct
		if(!SetupDiGetDeviceInterfaceDetail(dev_info, &didata, didetail,
											req_size, &req_size, NULL)) {
			WARN(_T("couldn't get devinterface info for %u"), index);
			break;
			}

		// open a shared handle to the device to query it (this will succeed even
		//  if the wiimote is already Connect()'ed)
		DEEP_TRACE(_T(".. querying device %s"), didetail->DevicePath);
		Handle = CreateFile(didetail->DevicePath, 0, FILE_SHARE_READ|FILE_SHARE_WRITE,
												  NULL, OPEN_EXISTING, 0, NULL);
		if(Handle == INVALID_HANDLE_VALUE) {
			DEEP_TRACE(_T(".... failed with err %x (probably harmless)."), 
					   GetLastError());
			goto skip;
			}
	
		// get the device attributes
		HIDD_ATTRIBUTES attrib;
		attrib.Size = sizeof(attrib);
		if(HidD_GetAttributes(Handle, &attrib))
			{
			// is this a wiimote?
			if((attrib.VendorID != VID) || (attrib.ProductID != PID))
				goto skip;

			// yes, but is it the one we're interested in?
			++wiimotes_found;
			if((wiimote_index != FIRST_AVAILABLE) &&
			   (wiimote_index != wiimotes_found))
				goto skip;

			// the wiimote is installed, but it may not be currently paired:
			if(wiimote_index == FIRST_AVAILABLE)
				TRACE(_T(".. opening Wiimote %u:"), wiimotes_found);
			else
				TRACE(_T(".. opening:"));


			// re-open the handle, but this time we don't allow write sharing
			//  (that way subsequent calls can still _discover_ wiimotes above, but
			//   will correctly fail here if they're already connected)
			CloseHandle(Handle);
			
			// note this also means that if another application has already opened
			//  the device, the library can no longer connect it (this may happen
			//  with software that enumerates all joysticks in the system, because
			//  even though the wiimote is not a standard joystick (and can't
			//  be read as such), it unfortunately announces itself to the OS
			//  as one.  The SDL library was known to do grab wiimotes like this.
			//  If you cannot stop the application from doing it, you may change the
			//  call below to open the device in full shared mode - but then the
			//  library can no longer detect if you've already connected a device
			//  and will allow you to connect it twice!  So be careful ...
			Handle = CreateFile(didetail->DevicePath, GENERIC_READ|GENERIC_WRITE,
													FILE_SHARE_READ| FILE_SHARE_WRITE,
													NULL, OPEN_EXISTING,
													FILE_FLAG_OVERLAPPED, NULL);
			if(Handle == INVALID_HANDLE_VALUE) {
				TRACE(_T(".... failed with err %x"), GetLastError());
				goto skip;
				}

			// clear the wiimote state & buffers
			Clear		  (false);		// preserves existing deadzones
			Internal.Clear(false);		// "
			InternalChanged = NO_CHANGE;
			memset(ReadBuff , 0, sizeof(ReadBuff));
			bConnectionLost	   = false;
			bConnectInProgress = true; // don't parse extensions or request regular
									   //  updates until complete
			// enable async reading
			BeginAsyncRead();

			// autodetect which write method the Bluetooth stack supports,
			//  by requesting the wiimote status report:

			if(force_hidwrites)
				TRACE(_T(".. (HID writes forced)"));
			else{
				//  - try WriteFile() first as it's the most efficient (it uses
				//     harware interrupts where possible and is async-capable):
				bUseHIDwrite = false;
				RequestStatusReport();
				//  and wait for the report to arrive:
				DWORD last_time = timeGetTime();
				while(!bStatusReceived && ((timeGetTime()-last_time) < 500))
					Sleep(10);
				TRACE(_T(".. WriteFile() %s."), bStatusReceived? _T("succeeded") :
																 _T("failed"));
				}

			// try HID write method (if supported)
			if(!bStatusReceived)
				{
				bUseHIDwrite = true;
				RequestStatusReport();
				// wait for the report to arrive:
				DWORD last_time = timeGetTime();
				while(!bStatusReceived && ((timeGetTime()-last_time) < 500))
					Sleep(10);
				// did we get it?
				TRACE(_T(".. HID write %s."), bStatusReceived? _T("succeeded") :
															   _T("failed"));
				}

			// still failed?
			if(!bStatusReceived) {
				WARN(_T("output failed - wiimote is not connected (or confused)."));
				Disconnect();
				goto skip;
				}

//Sleep(500);
			// reset it
			Reset();

			// read the wiimote calibration info
			ReadCalibration();

			// allow the result(s) to come in (so that the caller can immediately test
			//  MotionPlusConnected()
			Sleep(300); // note, don't need it on my system, better to be safe though

			// connected succesfully:
			_TotalConnected++;

			// use the first incomding analogue sensor values as the 'at rest'
			//  offsets (only supports the Balance Board currently)
			bCalibrateAtRest = true;

			// refresh the public state from the internal one (so that everything
			//  is available straight away
			RefreshState();

			// attempt to construct a unique hardware ID from the calibration
			//  data bytes (this is obviously not guaranteed to be unique across
			//  all devices, but may work fairly well in practice... ?)
			memcpy(&UniqueID, &CalibrationInfo, sizeof(CalibrationInfo));

			_ASSERT(UniqueID != 0); // if this fires, the calibration data didn't
									//  arrive - this shouldn't happen

#ifdef ID2_FROM_DEVICEPATH		// (see comments in header)
			// create a 2nd alternative id by simply adding all the characters
			//  in the device path to create a single number
			UniqueID2 = 0;
			for(unsigned index=0; index<_tcslen(didetail->DevicePath); index++)
				UniqueID2 += didetail->DevicePath[index];
#endif
			// and show when we want to trigger the next periodic status request
			//  (for battery level and connection loss detection)
			NextStatusTime		= timeGetTime() + REQUEST_STATUS_EVERY_MS;
			NextMPlusDetectTime = timeGetTime() + DETECT_MPLUS_EVERY_MS;
			MPlusDetectCount	= DETECT_MPLUS_COUNT;

			// tidy up
			delete[] (BYTE*)didetail;
			break;
			}
skip:
		// tidy up
		delete[] (BYTE*)didetail;

		if(Handle != INVALID_HANDLE_VALUE) {
			CloseHandle(Handle);
			Handle = INVALID_HANDLE_VALUE;
			}
		// if this was the specified wiimote index, abort
		if((wiimote_index != FIRST_AVAILABLE) &&
		   (wiimote_index == (wiimotes_found-1)))
		   break;

		index++;
		}

	// clean up our list
	SetupDiDestroyDeviceInfoList(dev_info);

	bConnectInProgress = false;
	if(IsConnected()) {
		TRACE(_T(".. connected!"));
		// notify the callbacks (if requested to do so)
		if(CallbackTriggerFlags & CONNECTED)
			{
			ChangedNotifier(CONNECTED, Internal);
			if(ChangedCallback)
				ChangedCallback(*this, CONNECTED, Internal);
			}
		return true;
		}
	TRACE(_T(".. connection failed."));
	return false;
	}
// ------------------------------------------------------------------------------------
void wiimote::CalibrateAtRest ()
	{
	_ASSERT(IsConnected());
	if(!IsConnected())
		return;

	// the app calls this to remove 'at rest' offsets from the analogue sensor
	//  values (currently only works for the Balance Board):
	if(IsBalanceBoard()) {
		TRACE(_T(".. removing 'at rest' BBoard offsets."));
		Internal.BalanceBoard.AtRestKg = Internal.BalanceBoard.Kg;
		RefreshState();
		}
	}
// ------------------------------------------------------------------------------------
void wiimote::Disconnect ()
	{
	if(Handle == INVALID_HANDLE_VALUE)
		return;

	TRACE(_T("Disconnect()."));
	
	if(IsConnected())
		{
		_ASSERT(_TotalConnected > 0); // sanity
		_TotalConnected--;
		
		if(!bConnectionLost)
			Reset();
		}

	CloseHandle(Handle);
	Handle = INVALID_HANDLE_VALUE;
	UniqueID  = 0;
#ifdef ID2_FROM_DEVICEPATH		// (see comments in header)
	UniqueID2 = 0;
#endif

	// close the read thread
	if(ReadParseThread) {
		// unblock it so it can realise we're closing and exit straight away
		SetEvent(DataRead);
		WaitForSingleObject(ReadParseThread, 3000);
		CloseHandle(ReadParseThread);
		ReadParseThread	= NULL;
		}
	// close the rumble thread
	if(AsyncRumbleThread) {
		WaitForSingleObject(AsyncRumbleThread, 3000);
		CloseHandle(AsyncRumbleThread);
		AsyncRumbleThread  = NULL;
		AsyncRumbleTimeout = 0;
		}
	// and the sample streaming thread
	if(SampleThread) {
		WaitForSingleObject(SampleThread, 3000);
		CloseHandle(SampleThread);
		SampleThread = NULL;
		}

#ifndef USE_DYNAMIC_HIDQUEUE
	HID.Deallocate();
#endif

	bStatusReceived = false;

	// and clear the state
	Clear		  (false); // (preserves deadzones)
	Internal.Clear(false); // "
	InternalChanged = NO_CHANGE;
	}
// ------------------------------------------------------------------------------------
void wiimote::Reset ()
	{
	TRACE(_T("Resetting wiimote."));
	
	if(bMotionPlusEnabled)
		DisableMotionPlus();

	// stop updates (by setting report type to non-continuous, buttons-only)
	if(IsBalanceBoard())
		SetReportType(IN_BUTTONS_BALANCE_BOARD, false);
	else
		SetReportType(IN_BUTTONS, false);

	SetRumble	 (false);
	SetLEDs		 (0x00);
//	MuteSpeaker  (true);
	EnableSpeaker(false);

	Sleep(150); // avoids loosing the extension calibration data on Connect()
	}
// ------------------------------------------------------------------------------------
unsigned __stdcall wiimote::ReadParseThreadfunc (void* param)
	{
	// this thread waits for the async ReadFile() to deliver data & parses it.
	//  it also requests periodic status updates, deals with connection loss
	//  and ends state recordings with a specific duration:
	_ASSERT(param);
	wiimote    &remote	   = *(wiimote*)param;
	OVERLAPPED &overlapped = remote.Overlapped;
	unsigned exit_code	   = 0; // (success)

	while(1)
		{
		// wait until the overlapped read completes, or the timeout is reached:
		DWORD wait = WaitForSingleObject(overlapped.hEvent, 500);

		// before we deal with the result, let's do some housekeeping:

		//  if we were recently Disconect()ed, exit now
		if(remote.Handle == INVALID_HANDLE_VALUE) {
			DEEP_TRACE(_T("read thread: wiimote was disconnected"));
			break;
			}
		//  ditto if the connection was lost (eg. through a failed write)
		if(remote.bConnectionLost)
			{
connection_lost:
			TRACE(_T("read thread: connection to wiimote was lost"));
			remote.Disconnect();
			remote.InternalChanged = (state_change_flags)
								(remote.InternalChanged | CONNECTION_LOST);
			// report via the callback (if any)
			if(remote.CallbackTriggerFlags & CONNECTION_LOST)
				{
				remote.ChangedNotifier(CONNECTION_LOST, remote.Internal);
				if(remote.ChangedCallback)
					remote.ChangedCallback(remote, CONNECTION_LOST, remote.Internal);
				}
			break;
			}

		DWORD time = timeGetTime();
		//  periodic events (but not if we're streaming audio,
		//					 we don't want to cause a glitch)
		if(remote.IsConnected() && !remote.bInitInProgress &&
		   !remote.IsPlayingAudio())
			{
			// status request due? 
			if(time > remote.NextStatusTime)
				{
#ifdef BEEP_ON_PERIODIC_STATUSREFRESH
				Beep(2000,50);
#endif
				remote.RequestStatusReport();
				// and schedule the next one
				remote.NextStatusTime = time + REQUEST_STATUS_EVERY_MS;
				}
			// motion plus detection due?
			if(!remote.IsBalanceBoard()		&&
//			   !remote.bConnectInProgress   &&
			   !remote.bMotionPlusExtension &&
			   (remote.Internal.ExtensionType != MOTION_PLUS) &&
			   (remote.Internal.ExtensionType != PARTIALLY_INSERTED) &&
			   (time > remote.NextMPlusDetectTime))
				{
				remote.DetectMotionPlusExtensionAsync();
				// we try several times in quick succession before the next
				//  delay:
				if(--remote.MPlusDetectCount == 0) {
					remote.NextMPlusDetectTime = time + DETECT_MPLUS_EVERY_MS;
					remote.MPlusDetectCount    = DETECT_MPLUS_COUNT;
#ifdef _DEBUG
					TRACE(_T("--"));
#endif
					}
				}
			}

		//  if we're state recording and have reached the specified duration, stop
		if(remote.Recording.bEnabled && (remote.Recording.EndTimeMS != UNTIL_STOP) &&
		   (time >= remote.Recording.EndTimeMS))
		   remote.Recording.bEnabled = false;

		// now handle the wait result:
		//  did the wait time out?
		if(wait == WAIT_TIMEOUT) {
			DEEP_TRACE(_T("read thread: timed out"));
			continue; // wait again
			}
		//  did an error occurr?
		if(wait != WAIT_OBJECT_0) {
			DEEP_TRACE(_T("read thread: error waiting!"));
			remote.bConnectionLost = true;
			// deal with it straight away to avoid a longer delay
			goto connection_lost;
			}
	
		// data was received:
#ifdef BEEP_DEBUG_READS
		Beep(500,1);
#endif
		DWORD read = 0;
		//  get the data read result
		GetOverlappedResult(remote.Handle, &overlapped, &read, TRUE);
		//  if we read data, parse it
		if(read) {
			DEEP_TRACE(_T("read thread: parsing data"));
			remote.OnReadData(read);
			}
		else
			DEEP_TRACE(_T("read thread: didn't get any data??"));
		}

	TRACE(_T("(ending read thread)"));
#ifdef BEEP_DEBUG_READS
	if(exit_code != 0)
		Beep(200,1000);
#endif
	return exit_code;
	}
// ------------------------------------------------------------------------------------
bool wiimote::BeginAsyncRead ()
	{
	// (this is also called before we're fully connected)
	if(Handle == INVALID_HANDLE_VALUE)
		return false;

	DEEP_TRACE(_T(".. starting async read"));
#ifdef BEEP_DEBUG_READS
	Beep(1000,1);
#endif

	DWORD read;
	if (!ReadFile(Handle, ReadBuff, REPORT_LENGTH, &read, &Overlapped)) {
		DWORD err = GetLastError();
		if(err != ERROR_IO_PENDING) {
			DEEP_TRACE(_T(".... ** ReadFile() failed! **"));
			return false;
			}
		}

	// launch the completion wait/callback thread
	if(!ReadParseThread) {
		ReadParseThread = (HANDLE)_beginthreadex(NULL, 0, ReadParseThreadfunc,
												 this, 0, NULL);
		DEEP_TRACE(_T(".... creating read thread"));
		_ASSERT(ReadParseThread);
		if(!ReadParseThread)
			return false;
		SetThreadPriority(ReadParseThread, WORKER_THREAD_PRIORITY);
		}

	// if ReadFile completed while we called, signal the thread to proceed
	if(read) {
		DEEP_TRACE(_T(".... got data right away"));
		SetEvent(DataRead);
		}
	return true;
	}
// ------------------------------------------------------------------------------------
void wiimote::OnReadData (DWORD bytes_read)
	{
	_ASSERT(bytes_read == REPORT_LENGTH);

	// copy our input buffer
	BYTE buff [REPORT_LENGTH];
	memcpy(buff, ReadBuff, bytes_read);

	// start reading again
	BeginAsyncRead();

	// parse it
	ParseInput(buff);
	}
// ------------------------------------------------------------------------------------
void wiimote::SetReportType (input_report type, bool continuous)
	{
	_ASSERT(IsConnected());
	if(!IsConnected())
		return;

	// the balance board only uses one type of report
	_ASSERT(!IsBalanceBoard() || type == IN_BUTTONS_BALANCE_BOARD);
	if(IsBalanceBoard() && (type != IN_BUTTONS_BALANCE_BOARD))
		return;

#ifdef TRACE
	#define TYPE2NAME(_type)	(type==_type)? _T(#_type)
	const TCHAR* name = TYPE2NAME(IN_BUTTONS)				:
						TYPE2NAME(IN_BUTTONS_ACCEL_IR)		:
						TYPE2NAME(IN_BUTTONS_ACCEL_EXT)		:
						TYPE2NAME(IN_BUTTONS_ACCEL_IR_EXT)	:
						TYPE2NAME(IN_BUTTONS_BALANCE_BOARD) :
						_T("(unknown?)");
	TRACE(_T("ReportType: %s (%s)"), name, (continuous? _T("continuous") :
														_T("non-continuous")));
#endif
	ReportType = type;

	switch(type)
		{
		case IN_BUTTONS_ACCEL_IR:
			EnableIR(wiimote_state::ir::EXTENDED);
			break;
		case IN_BUTTONS_ACCEL_IR_EXT:
			EnableIR(wiimote_state::ir::BASIC);
			break;
		default:
			DisableIR();
			break;
		}

	BYTE buff [REPORT_LENGTH] = {0};
	buff[0] = OUT_TYPE;
	buff[1] = (continuous ? 0x04 : 0x00) | GetRumbleBit();
	buff[2] = (BYTE)type;
	WriteReport(buff);
//	Sleep(15);
	}
// ------------------------------------------------------------------------------------
void wiimote::SetLEDs (BYTE led_bits)
	{
	_ASSERT(IsConnected());
	if(!IsConnected() || bInitInProgress)
		return;

	_ASSERT(led_bits <= 0x0f);
	led_bits &= 0xf;
	
	BYTE buff [REPORT_LENGTH] = {0};
	buff[0] = OUT_LEDs;
	buff[1] = (led_bits<<4) | GetRumbleBit();
	WriteReport(buff);

	Internal.LED.Bits = led_bits;
	}
// ------------------------------------------------------------------------------------
void wiimote::SetRumble (bool on)
	{
	_ASSERT(IsConnected());
	if(!IsConnected())
		return;

	if(Internal.bRumble == on)
		return;

	Internal.bRumble = on;

	// if we're streaming audio, we don't need to send a report (sending it makes
	// the audio glitch, and the rumble bit is sent with every report anyway)
	if(IsPlayingAudio())
		return;

	BYTE buff [REPORT_LENGTH] = {0};
	buff[0] = OUT_STATUS;
	buff[1] = on? 0x01 : 0x00;
	WriteReport(buff);
	}
// ------------------------------------------------------------------------------------
unsigned __stdcall wiimote::AsyncRumbleThreadfunc (void* param)
	{
	// auto-disables rumble after x milliseconds:
	_ASSERT(param);
	wiimote &remote = *(wiimote*)param;
	
	while(remote.IsConnected())
		{
		if(remote.AsyncRumbleTimeout)
			{
			DWORD current_time = timeGetTime();
			if(current_time >= remote.AsyncRumbleTimeout)
				{
				if(remote.Internal.bRumble)
					remote.SetRumble(false);
				remote.AsyncRumbleTimeout = 0;
				}
			Sleep(1);
			}
		else
			Sleep(4);
		}
	return 0;
	}
// ------------------------------------------------------------------------------------
void wiimote::RumbleForAsync (unsigned milliseconds)
	{
	// rumble for a fixed amount of time
	_ASSERT(IsConnected());
	if(!IsConnected())
		return;

	SetRumble(true);

	// show how long thread should wait to disable rumble again
	// (it it's currently rumbling it will just extend the time)
	AsyncRumbleTimeout = timeGetTime() + milliseconds;

	// create the thread?
	if(AsyncRumbleThread)
		return;

	AsyncRumbleThread = (HANDLE)_beginthreadex(NULL, 0, AsyncRumbleThreadfunc, this,
											   0, NULL);
	_ASSERT(AsyncRumbleThread);
	if(!AsyncRumbleThread) {
		WARN(_T("couldn't create rumble thread!"));
		return;
		}
	SetThreadPriority(AsyncRumbleThread, WORKER_THREAD_PRIORITY);
	}
// ------------------------------------------------------------------------------------
void wiimote::RequestStatusReport ()
	{
	// (this can be called before we're fully connected)
	_ASSERT(Handle != INVALID_HANDLE_VALUE);
	if(Handle == INVALID_HANDLE_VALUE)
		return;

	BYTE buff [REPORT_LENGTH] = {0};
	buff[0] = OUT_STATUS;
	buff[1] = GetRumbleBit();
	WriteReport(buff);
	}
// ------------------------------------------------------------------------------------
bool wiimote::ReadAddress (int address, short size)
	{
	// asynchronous
	BYTE buff [REPORT_LENGTH] = {0};
	buff[0] = OUT_READMEMORY;
	buff[1] = (BYTE)(((address & 0xff000000) >> 24) | GetRumbleBit());
	buff[2] = (BYTE)( (address & 0x00ff0000) >> 16);
	buff[3] = (BYTE)( (address & 0x0000ff00) >>  8);
	buff[4] = (BYTE)( (address & 0x000000ff));
	buff[5] = (BYTE)( (size	   & 0xff00	   ) >>  8);
	buff[6] = (BYTE)( (size	   & 0xff));
	return WriteReport(buff);
	}
// ------------------------------------------------------------------------------------
void wiimote::WriteData (int address, BYTE size, const BYTE* buff)
	{
	// asynchronous
	BYTE write [REPORT_LENGTH] = {0};
	write[0] = OUT_WRITEMEMORY;
	write[1] = (BYTE)(((address & 0xff000000) >> 24) | GetRumbleBit());
	write[2] = (BYTE)( (address & 0x00ff0000) >> 16);
	write[3] = (BYTE)( (address & 0x0000ff00) >>  8);
	write[4] = (BYTE)( (address & 0x000000ff));
	write[5] = size;
	memcpy(write+6, buff, size);
	WriteReport(write);
	}
// ------------------------------------------------------------------------------------
int wiimote::ParseInput (BYTE* buff)
	{
	int changed = 0;

	// lock our internal state (so RefreshState() is blocked until we're done
	EnterCriticalSection(&StateLock);

	switch(buff[0])
		{
		case IN_BUTTONS:
			DEEP_TRACE(_T(".. parsing buttons."));
			changed |= ParseButtons(buff);
			break;

		case IN_BUTTONS_ACCEL:
			DEEP_TRACE(_T(".. parsing buttons/accel."));
			changed |= ParseButtons(buff);
			if(!IsBalanceBoard())
				changed |= ParseAccel(buff);
			break;

		case IN_BUTTONS_ACCEL_EXT:
			DEEP_TRACE(_T(".. parsing extenion/accel."));
			changed |= ParseButtons(buff);
			changed |= ParseExtension(buff, 6);
			if(!IsBalanceBoard())
				changed |= ParseAccel(buff);
			break;

		case IN_BUTTONS_ACCEL_IR:
			DEEP_TRACE(_T(".. parsing ir/accel."));
			changed |= ParseButtons(buff);
			if(!IsBalanceBoard()) {
				changed |= ParseAccel(buff);
				changed |= ParseIR(buff);
				}
			break;

		case IN_BUTTONS_ACCEL_IR_EXT:
			DEEP_TRACE(_T(".. parsing ir/extenion/accel."));
			changed |= ParseButtons(buff);
			changed |= ParseExtension(buff, 16);
			if(!IsBalanceBoard()) {
				changed |= ParseAccel(buff);
				changed |= ParseIR	   (buff);
				}
			break;

		case IN_BUTTONS_BALANCE_BOARD:
			DEEP_TRACE(_T(".. parsing buttson/balance."));
			changed |= ParseButtons(buff);
			changed |= ParseExtension(buff, 3);
			break;

		case IN_READADDRESS:
			DEEP_TRACE(_T(".. parsing read address."));
			changed |= ParseButtons	   (buff);
			changed |= ParseReadAddress(buff);
			break;

		case IN_STATUS:
			DEEP_TRACE(_T(".. parsing status."));
			changed |= ParseStatus(buff);
			// show that we received the status report (used for output method
			//  detection during Connect())
			bStatusReceived = true;
			break;
		
		default:
			DEEP_TRACE(_T(".. ** unknown input ** (happens)."));
			///_ASSERT(0);
			//Debug.WriteLine("Unknown report type: " + type.ToString());
			LeaveCriticalSection(&StateLock);
			return false;
		}

	// if we're recording and some state we care about has changed, insert it into
	//  the state history
	if(Recording.bEnabled && (changed & Recording.TriggerFlags))
		{
		DEEP_TRACE(_T(".. adding state to history"));
		state_event event;
		event.time_ms = timeGetTime();
		event.state	  = *(wiimote_state*)this;
		Recording.StateHistory->push_back(event);
		}

	// for polling: show which state has changed since the last RefreshState()
	InternalChanged = (state_change_flags)(InternalChanged | changed);

	LeaveCriticalSection(&StateLock);

	// callbacks: call it (if set & state the app is interested in has changed)
	if(changed & CallbackTriggerFlags)
		{
		DEEP_TRACE(_T(".. calling state change callback"));
		ChangedNotifier((state_change_flags)changed, Internal);
		if(ChangedCallback)
			ChangedCallback(*this, (state_change_flags)changed, Internal);
		}
	
	DEEP_TRACE(_T(".. parse complete."));
	return true;
	}
// ------------------------------------------------------------------------------------
state_change_flags wiimote::RefreshState ()
	{
	// nothing changed since the last call?
	if(InternalChanged == NO_CHANGE)
		return NO_CHANGE;

	// copy the internal state to our public data members:
	//  synchronise the interal state with the read/parse thread (we don't want
	//   values changing during the copy)
	EnterCriticalSection(&StateLock);
	
	// remember which state changed since the last call
	state_change_flags changed = InternalChanged;
	
	// preserve the application-set deadzones (if any)
	joystick::deadzone nunchuk_deadzone	     = Nunchuk.Joystick.DeadZone;
	joystick::deadzone classic_joyl_deadzone = ClassicController.JoystickL.DeadZone;
	joystick::deadzone classic_joyr_deadzone = ClassicController.JoystickR.DeadZone;
		
	 // copy the internal state to the public one
	*(wiimote_state*)this = Internal;
	InternalChanged		  = NO_CHANGE;
	 
	 // restore the application-set deadzones
	Nunchuk.Joystick.DeadZone			 = nunchuk_deadzone;
	ClassicController.JoystickL.DeadZone = classic_joyl_deadzone;
	ClassicController.JoystickR.DeadZone = classic_joyr_deadzone;

	LeaveCriticalSection(&StateLock);
	
	return changed;
	}
// ------------------------------------------------------------------------------------
void wiimote::DetectMotionPlusExtensionAsync ()
	{
#ifdef _DEBUG
	TRACE(_T("(looking for motion plus)"));
#endif
	// show that we're expecting the result shortly
	MotionPlusDetectCount++;
	// MotionPLus reports at a different address than other extensions (until
	//  activated, when it maps itself into the usual extension registers), so
	//  try to detect it first:
	ReadAddress(REGISTER_MOTIONPLUS_DETECT, 6);
	}
// ------------------------------------------------------------------------------------
bool wiimote::EnableMotionPlus ()
	{
	_ASSERT(bMotionPlusDetected);
	if(!bMotionPlusDetected)
		return false;
	if(bMotionPlusEnabled)
		return true;

	TRACE(_T("Enabling Motion Plus:"));
	
	bMotionPlusExtension = false;
	bInitInProgress		 = true;
	bEnablingMotionPlus	 = true;

	// Initialize it:
	WriteData(REGISTER_MOTIONPLUS_INIT  , 0x55);
//	Sleep(50);
	// Enable it (this maps it to the standard extension port):
	WriteData(REGISTER_MOTIONPLUS_ENABLE, 0x04);
//	Sleep(50);
Sleep(500);
	return true;
	}
// ------------------------------------------------------------------------------------
bool wiimote::DisableMotionPlus ()
	{
 	if(!bMotionPlusDetected || !bMotionPlusEnabled)
		return false;

	TRACE(_T("Disabling Motion Plus:"));

	// disable it (this makes standard extensions visible again)
	WriteData(REGISTER_EXTENSION_INIT1, 0x55);
	return true;
	}
// ------------------------------------------------------------------------------------
void wiimote::InitializeExtension ()
	{
	TRACE(_T("Initialising Extension."));
	// wibrew.org: The new way to initialize the extension is by writing 0x55 to
	//	0x(4)A400F0, then writing 0x00 to 0x(4)A400FB. It works on all extensions, and
	//  makes the extension type bytes unencrypted. This means that you no longer have
	//  to decrypt the extension bytes using the transform listed above. 
	bInitInProgress = true;
_ASSERT(Internal.bExtension);
	// only initialize if it's not a MotionPlus
	if(!bEnablingMotionPlus) {
		WriteData  (REGISTER_EXTENSION_INIT1, 0x55);
		WriteData  (REGISTER_EXTENSION_INIT2, 0x00);
		}
	else
		bEnablingMotionPlus = false;
	
	ReadAddress(REGISTER_EXTENSION_TYPE , 6);
	}
// ------------------------------------------------------------------------------------
int wiimote::ParseStatus (BYTE* buff)
	{
	// parse the buttons
	int changed = ParseButtons(buff);
			
	// get the battery level
	BYTE battery_raw = buff[6];
	if(Internal.BatteryRaw != battery_raw)
		changed |= BATTERY_CHANGED;
	Internal.BatteryRaw	 = battery_raw;
	// it is estimated that ~200 is the maximum battery level
	Internal.BatteryPercent = battery_raw / 2;

	// there is also a flag that shows if the battery is nearly empty
	bool drained = buff[3] & 0x01;
	if(drained != bBatteryDrained)
		{
		bBatteryDrained = drained;
		if(drained)
			changed |= BATTERY_DRAINED;
		}

	// leds
	BYTE leds = buff[3] >> 4;
	if(leds != Internal.LED.Bits)
		changed |= LEDS_CHANGED;
	Internal.LED.Bits = leds;

	// don't handle extensions until a connection is complete
//	if(bConnectInProgress)
//		return changed;

	bool extension = ((buff[3] & 0x02) != 0);
//	TRACE(_T("(extension = %s)"), (extension? _T("TRUE") : _T("false")));

	if(extension != Internal.bExtension)
		{
		if(!Internal.bExtension)
			{
			TRACE(_T("Extension connected:"));
			Internal.bExtension = true;
			InitializeExtension();
			}
		else{
			TRACE(_T("Extension disconnected."));
			Internal.bExtension	   = false;
			Internal.ExtensionType = wiimote_state::NONE;
			bMotionPlusEnabled	   = false;
			bMotionPlusExtension   = false;
			bMotionPlusDetected	   = false;
			bInitInProgress		   = false;
			bEnablingMotionPlus	   = false;
			changed				  |= EXTENSION_DISCONNECTED;
			// renable reports
//			SetReportType(ReportType);
			}
		}
	
	return changed;
	}
// ------------------------------------------------------------------------------------
int wiimote::ParseButtons (BYTE* buff)
	{
	int changed = 0;
	
//	WORD bits = *(WORD*)(buff+1);
	WORD bits = *(WORD*)(buff+1) & Button.ALL;

	if(bits != Internal.Button.Bits)
		changed |= BUTTONS_CHANGED;
	Internal.Button.Bits = bits;
	
	return changed;
	}
// ------------------------------------------------------------------------------------
bool wiimote::EstimateOrientationFrom (wiimote_state::acceleration &accel)
	{
	// Orientation estimate from acceleration data (shared between wiimote and nunchuk)
	//  return true if the orientation was updated

	//  assume the controller is stationary if the acceleration vector is near
	//  1g for several updates (this may not always be correct)
	float length_sq = square(accel.X) + square(accel.Y) + square(accel.Z);

	// TODO: as I'm comparing _squared_ length, I really need different
	//		  min/max epsilons...
	#define DOT(x1,y1,z1, x2,y2,z2)	((x1*x2) + (y1*y2) + (z1*z2))

	static const float epsilon = 0.2f;
	if((length_sq >= (1.f-epsilon)) && (length_sq <= (1.f+epsilon)))
		{
		if(++WiimoteNearGUpdates < 2)
			return false;
		
		// wiimote seems to be stationary:  normalize the current acceleration
		//  (ie. the assumed gravity vector)
		float inv_len = 1.f / sqrt(length_sq);
		float x = accel.X * inv_len;
		float y = accel.Y * inv_len;
		float z = accel.Z * inv_len;

		// copy the values
		accel.Orientation.X = x;
		accel.Orientation.Y = y;
		accel.Orientation.Z = z;

		// and extract pitch & roll from them:
		// (may not be optimal)
		float pitch = -asin(y)    * 57.2957795f;
//		float roll  =  asin(x)    * 57.2957795f;
        float roll  =  atan2(x,z) * 57.2957795f;
		if(z < 0) {
			pitch = (y < 0)?  180 - pitch : -180 - pitch;
			roll  = (x < 0)? -180 - roll  :  180 - roll;
			}

		accel.Orientation.Pitch = pitch;
		accel.Orientation.Roll  = roll;

		// show that we just updated orientation
		accel.Orientation.UpdateAge = 0;
#ifdef BEEP_ON_ORIENTATION_ESTIMATE
		Beep(2000, 1);
#endif
		return true; // updated
		}

	// not updated this time:
	WiimoteNearGUpdates	= 0;
	// age the last orientation update
	accel.Orientation.UpdateAge++;
	return false;
	}
// ------------------------------------------------------------------------------------
void wiimote::ApplyJoystickDeadZones (wiimote_state::joystick &joy)
	{
	// apply the deadzones to each axis (if set)
	if((joy.DeadZone.X > 0.f) && (joy.DeadZone.X <= 1.f))
		{
		if(fabs(joy.X) <= joy.DeadZone.X)
			joy.X = 0;
		else{
			joy.X -= joy.DeadZone.X * sign(joy.X);
			joy.X /= 1.f - joy.DeadZone.X;
			}
		}
	if((joy.DeadZone.Y > 0.f) && (joy.DeadZone.Y <= 1.f))
		{
		if(fabs(joy.Y) <= joy.DeadZone.Y)
			joy.Y = 0;
		else{
			joy.Y -= joy.DeadZone.Y * sign(joy.Y);
			joy.Y /= 1.f - joy.DeadZone.Y;
			}
		}
	}
// ------------------------------------------------------------------------------------
int wiimote::ParseAccel (BYTE* buff)
	{
	int changed = 0;
	
	BYTE raw_x = buff[3];
	BYTE raw_y = buff[4];
	BYTE raw_z = buff[5];

	if((raw_x != Internal.Acceleration.RawX) ||
	   (raw_y != Internal.Acceleration.RawY) ||
	   (raw_z != Internal.Acceleration.RawZ))
	   changed |= ACCEL_CHANGED;
	
	Internal.Acceleration.RawX = raw_x;
	Internal.Acceleration.RawY = raw_y;
	Internal.Acceleration.RawZ = raw_z;

	// avoid / 0.0 when calibration data hasn't arrived yet
	if(Internal.CalibrationInfo.X0)
		{
		Internal.Acceleration.X =
					((float)Internal.Acceleration.RawX  - Internal.CalibrationInfo.X0) / 
					((float)Internal.CalibrationInfo.XG - Internal.CalibrationInfo.X0);
		Internal.Acceleration.Y =
					((float)Internal.Acceleration.RawY  - Internal.CalibrationInfo.Y0) /
					((float)Internal.CalibrationInfo.YG - Internal.CalibrationInfo.Y0);
		Internal.Acceleration.Z =
					((float)Internal.Acceleration.RawZ  - Internal.CalibrationInfo.Z0) /
					((float)Internal.CalibrationInfo.ZG - Internal.CalibrationInfo.Z0);
		}
	else{
		Internal.Acceleration.X =
		Internal.Acceleration.Y =
		Internal.Acceleration.Z = 0.f;
		}

	// see if we can estimate the orientation from the current values
	if(EstimateOrientationFrom(Internal.Acceleration))
		changed |= ORIENTATION_CHANGED;
	
	return changed;
	}
// ------------------------------------------------------------------------------------
int wiimote::ParseIR (BYTE* buff)
	{
	if(Internal.IR.Mode == wiimote_state::ir::OFF)
		return NO_CHANGE;

	// avoid garbage values when the MotionPlus is enabled, but the app is
	//  still using the extended IR report type
	if(bMotionPlusEnabled && (Internal.IR.Mode == wiimote_state::ir::EXTENDED))
		return NO_CHANGE;

	// take a copy of the existing IR state (so we can detect changes)
	wiimote_state::ir prev_ir = Internal.IR;

	// only updates the other values if the dots are visible (so that the last
	//  valid values stay unmodified)
	switch(Internal.IR.Mode)
		{
		case wiimote_state::ir::BASIC:
			// 2 dots are encoded in 5 bytes, so read 2 at a time
			for(unsigned step=0; step<2; step++)
				{
				ir::dot &dot0 = Internal.IR.Dot[step*2  ];
				ir::dot &dot1 = Internal.IR.Dot[step*2+1];
				const unsigned offs = 6 + (step*5); // 5 bytes for 2 dots

				dot0.bVisible = !(buff[offs  ] == 0xff && buff[offs+1] == 0xff);
				dot1.bVisible = !(buff[offs+3] == 0xff && buff[offs+4] == 0xff);
			
				if(dot0.bVisible) {
					dot0.RawX = buff[offs  ] | ((buff[offs+2] >> 4) & 0x03) << 8;;
					dot0.RawY = buff[offs+1] | ((buff[offs+2] >> 6) & 0x03) << 8;;
					dot0.X    = 1.f - (dot0.RawX / (float)wiimote_state::ir::MAX_RAW_X);
					dot0.Y    =	      (dot0.RawY / (float)wiimote_state::ir::MAX_RAW_Y);
					}
				if(dot1.bVisible) {
					dot1.RawX = buff[offs+3] | ((buff[offs+2] >> 0) & 0x03) << 8;
					dot1.RawY = buff[offs+4] | ((buff[offs+2] >> 2) & 0x03) << 8;
					dot1.X    = 1.f - (dot1.RawX / (float)wiimote_state::ir::MAX_RAW_X);
					dot1.Y    =	      (dot1.RawY / (float)wiimote_state::ir::MAX_RAW_Y);
					}
				}
			break;
		
		case wiimote_state::ir::EXTENDED:
			// each dot is encoded into 3 bytes
			for(unsigned index=0; index<4; index++)
				{
				ir::dot &dot = Internal.IR.Dot[index];
				const unsigned offs = 6 + (index * 3);
			
				dot.bVisible = !(buff[offs  ]==0xff && buff[offs+1]==0xff &&
							   buff[offs+2]==0xff);
				if(dot.bVisible) {
					dot.RawX = buff[offs  ] | ((buff[offs+2] >> 4) & 0x03) << 8;
					dot.RawY = buff[offs+1] | ((buff[offs+2] >> 6) & 0x03) << 8;
					dot.X    = 1.f - (dot.RawX / (float)wiimote_state::ir::MAX_RAW_X);
					dot.Y    =	     (dot.RawY / (float)wiimote_state::ir::MAX_RAW_Y);
					dot.Size = buff[offs+2] & 0x0f;
					}
				}
			break;

		case wiimote_state::ir::FULL:
			_ASSERT(0); // not supported yet;
			break;
		}

	return memcmp(&prev_ir, &Internal.IR, sizeof(Internal.IR))? IR_CHANGED : 0;
	}
// ------------------------------------------------------------------------------------
inline float wiimote::GetBalanceValue (short sensor, short min, short mid, short max)
	{
	if(max == mid || mid == min)
		return 0;

	float val = (sensor < mid)?
					68.0f * ((float)(sensor - min) / (mid - min)) :
					68.0f * ((float)(sensor - mid) / (max - mid)) + 68.0f;
	
	// divide by four (so that each sensor is correct)
	return val * 0.25f;
	}
// ------------------------------------------------------------------------------------
int wiimote::ParseExtension (BYTE *buff, unsigned offset)
	{
	int changed = 0;
	
	switch(Internal.ExtensionType)
		{
		case wiimote_state::NUNCHUK:
			{
			// buttons
			bool c = (buff[offset+5] & 0x02) == 0;
			bool z = (buff[offset+5] & 0x01) == 0;
			
			if((c != Internal.Nunchuk.C) || (z != Internal.Nunchuk.Z))
				changed |= NUNCHUK_BUTTONS_CHANGED;
			
			Internal.Nunchuk.C = c;
			Internal.Nunchuk.Z = z;

			// acceleration
			{
			wiimote_state::acceleration &accel = Internal.Nunchuk.Acceleration;
			
			BYTE raw_x = buff[offset+2];
			BYTE raw_y = buff[offset+3];
			BYTE raw_z = buff[offset+4];
			if((raw_x != accel.RawX) || (raw_y != accel.RawY) || (raw_z != accel.RawZ))
				changed |= NUNCHUK_ACCEL_CHANGED;

			accel.RawX = raw_x;
			accel.RawY = raw_y;
			accel.RawZ = raw_z;

			wiimote_state::nunchuk::calibration_info &calib =
													Internal.Nunchuk.CalibrationInfo;
			accel.X = ((float)raw_x - calib.X0) / ((float)calib.XG - calib.X0);
			accel.Y = ((float)raw_y - calib.Y0) / ((float)calib.YG - calib.Y0);
			accel.Z = ((float)raw_z - calib.Z0) / ((float)calib.ZG - calib.Z0);

			// try to extract orientation from the accel:
			if(EstimateOrientationFrom(accel))
				changed |= NUNCHUK_ORIENTATION_CHANGED;
			}
			{
			// joystick:
			wiimote_state::joystick &joy = Internal.Nunchuk.Joystick;

			float raw_x = buff[offset+0];
			float raw_y = buff[offset+1];
			
			if((raw_x != joy.RawX) || (raw_y != joy.RawY))
				changed |= NUNCHUK_JOYSTICK_CHANGED;

			joy.RawX = raw_x;
			joy.RawY = raw_y;

			// apply the calibration data
			wiimote_state::nunchuk::calibration_info &calib =
													Internal.Nunchuk.CalibrationInfo;
			if(Internal.Nunchuk.CalibrationInfo.MaxX != 0x00)
				joy.X = ((float)raw_x - calib.MidX) / ((float)calib.MaxX - calib.MinX);
			if(calib.MaxY != 0x00)
				joy.Y = ((float)raw_y - calib.MidY) / ((float)calib.MaxY - calib.MinY);

			// i prefer the outputs to range -1 - +1 (note this also affects the
			//  deadzone calculations)
			joy.X *= 2;	joy.Y *= 2;

			// apply the public deadzones to the internal state (if set)
			joy.DeadZone = Nunchuk.Joystick.DeadZone;
			ApplyJoystickDeadZones(joy);
			}
			}
			break;
		
		case wiimote_state::CLASSIC:
		case wiimote_state::GH3_GHWT_GUITAR:
		case wiimote_state::GHWT_DRUMS:
			{
			// buttons:
			WORD bits = *(WORD*)(buff+offset+4);
			bits = ~bits; // need to invert bits since 0 is down, and 1 is up

			if(bits != Internal.ClassicController.Button.Bits)
				changed |= CLASSIC_BUTTONS_CHANGED;

			Internal.ClassicController.Button.Bits = bits;

			// joysticks:
			wiimote_state::joystick &joyL = Internal.ClassicController.JoystickL;
			wiimote_state::joystick &joyR = Internal.ClassicController.JoystickR;

			float l_raw_x = (float) (buff[offset+0] & 0x3f);
			float l_raw_y = (float) (buff[offset+1] & 0x3f);
			float r_raw_x = (float)((buff[offset+2]		    >> 7) |
								   ((buff[offset+1] & 0xc0) >> 5) |
								   ((buff[offset+0] & 0xc0) >> 3));
			float r_raw_y = (float) (buff[offset+2] & 0x1f);

			if((joyL.RawX != l_raw_x) || (joyL.RawY != l_raw_y))
				changed |= CLASSIC_JOYSTICK_L_CHANGED;
			if((joyR.RawX != r_raw_x) || (joyR.RawY != r_raw_y))
				changed |= CLASSIC_JOYSTICK_R_CHANGED;

			joyL.RawX = l_raw_x; joyL.RawY = l_raw_y;
			joyR.RawX = r_raw_x; joyR.RawY = r_raw_y;

			// apply calibration
			wiimote_state::classic_controller::calibration_info &calib =
											Internal.ClassicController.CalibrationInfo;
			if(calib.MaxXL != 0x00)
				joyL.X = (joyL.RawX - calib.MidXL) / ((float)calib.MaxXL - calib.MinXL);
			if(calib.MaxYL != 0x00)
				joyL.Y = (joyL.RawY - calib.MidYL) / ((float)calib.MaxYL - calib.MinYL);
			if(calib.MaxXR != 0x00)
				joyR.X = (joyR.RawX - calib.MidXR) / ((float)calib.MaxXR - calib.MinXR);
			if(calib.MaxYR != 0x00)
				joyR.Y = (joyR.RawY - calib.MidYR) / ((float)calib.MaxYR - calib.MinYR);

			// i prefer the joystick outputs to range -1 - +1 (note this also affects
			//  the deadzone calculations)
			joyL.X *= 2; joyL.Y *= 2; joyR.X *= 2; joyR.Y *= 2;

			// apply the public deadzones to the internal state (if set)
			joyL.DeadZone = ClassicController.JoystickL.DeadZone;
			joyR.DeadZone = ClassicController.JoystickR.DeadZone;
			ApplyJoystickDeadZones(joyL);
			ApplyJoystickDeadZones(joyR);

			// triggers
			BYTE raw_trigger_l = ((buff[offset+2] & 0x60) >> 2) |
								  (buff[offset+3]		  >> 5);
			BYTE raw_trigger_r =   buff[offset+3] & 0x1f;
			
			if((raw_trigger_l != Internal.ClassicController.RawTriggerL) ||
			   (raw_trigger_r != Internal.ClassicController.RawTriggerR))
			   changed |= CLASSIC_TRIGGERS_CHANGED;
			
			Internal.ClassicController.RawTriggerL  = raw_trigger_l;
			Internal.ClassicController.RawTriggerR  = raw_trigger_r;

			if(calib.MaxTriggerL != 0x00)
				Internal.ClassicController.TriggerL =
									 (float)Internal.ClassicController.RawTriggerL / 
									((float)calib.MaxTriggerL -	calib.MinTriggerL);
			if(calib.MaxTriggerR != 0x00)
				Internal.ClassicController.TriggerR =
									 (float)Internal.ClassicController.RawTriggerR / 
									((float)calib.MaxTriggerR - calib.MinTriggerR);
			}
			break;

		case BALANCE_BOARD:
			{
			wiimote_state::balance_board::sensors_raw prev_raw =
														Internal.BalanceBoard.Raw;
			Internal.BalanceBoard.Raw.TopR	  =
						(short)((short)buff[offset+0] << 8 | buff[offset+1]);
			Internal.BalanceBoard.Raw.BottomR =
						(short)((short)buff[offset+2] << 8 | buff[offset+3]);
			Internal.BalanceBoard.Raw.TopL	  =
						(short)((short)buff[offset+4] << 8 | buff[offset+5]);
			Internal.BalanceBoard.Raw.BottomL =
						(short)((short)buff[offset+6] << 8 | buff[offset+7]);

			if((Internal.BalanceBoard.Raw.TopL    != prev_raw.TopL)    ||
			   (Internal.BalanceBoard.Raw.TopR    != prev_raw.TopR)    ||
			   (Internal.BalanceBoard.Raw.BottomL != prev_raw.BottomL) ||
			   (Internal.BalanceBoard.Raw.BottomR != prev_raw.BottomR))
				changed |= BALANCE_WEIGHT_CHANGED;

			Internal.BalanceBoard.Kg.TopL	 =
				GetBalanceValue(Internal.BalanceBoard.Raw.TopL,
								Internal.BalanceBoard.CalibrationInfo.Kg0 .TopL,
								Internal.BalanceBoard.CalibrationInfo.Kg17.TopL,
								Internal.BalanceBoard.CalibrationInfo.Kg34.TopL);
			Internal.BalanceBoard.Kg.TopR	 =
				GetBalanceValue(Internal.BalanceBoard.Raw.TopR,
								Internal.BalanceBoard.CalibrationInfo.Kg0 .TopR,
								Internal.BalanceBoard.CalibrationInfo.Kg17.TopR,
								Internal.BalanceBoard.CalibrationInfo.Kg34.TopR);
			Internal.BalanceBoard.Kg.BottomL =
				GetBalanceValue(Internal.BalanceBoard.Raw.BottomL,
								Internal.BalanceBoard.CalibrationInfo.Kg0 .BottomL,
								Internal.BalanceBoard.CalibrationInfo.Kg17.BottomL,
								Internal.BalanceBoard.CalibrationInfo.Kg34.BottomL);
			Internal.BalanceBoard.Kg.BottomR =
				GetBalanceValue(Internal.BalanceBoard.Raw.BottomR,
								Internal.BalanceBoard.CalibrationInfo.Kg0 .BottomR,
								Internal.BalanceBoard.CalibrationInfo.Kg17.BottomR,
								Internal.BalanceBoard.CalibrationInfo.Kg34.BottomR);
			
			// uses these as the 'at rest' offsets? (immediately after Connect(),
			//  or if the app called CalibrateAtRest())
			if(bCalibrateAtRest) {
				bCalibrateAtRest = false;
				TRACE(_T(".. Auto-removing 'at rest' BBoard offsets."));
				Internal.BalanceBoard.AtRestKg = Internal.BalanceBoard.Kg;
				}

			// remove the 'at rest' offsets
			Internal.BalanceBoard.Kg.TopL	 -= BalanceBoard.AtRestKg.TopL;
			Internal.BalanceBoard.Kg.TopR	 -= BalanceBoard.AtRestKg.TopR;
			Internal.BalanceBoard.Kg.BottomL -= BalanceBoard.AtRestKg.BottomL;
			Internal.BalanceBoard.Kg.BottomR -= BalanceBoard.AtRestKg.BottomR;

			// compute the average
			Internal.BalanceBoard.Kg.Total	  = Internal.BalanceBoard.Kg.TopL    +
												Internal.BalanceBoard.Kg.TopR    +
												Internal.BalanceBoard.Kg.BottomL +
												Internal.BalanceBoard.Kg.BottomR;
			// and convert to Lbs
			const float KG2LB = 2.20462262f;
			Internal.BalanceBoard.Lb		  = Internal.BalanceBoard.Kg;
			Internal.BalanceBoard.Lb.TopL	 *= KG2LB;
			Internal.BalanceBoard.Lb.TopR	 *= KG2LB;
			Internal.BalanceBoard.Lb.BottomL *= KG2LB;
			Internal.BalanceBoard.Lb.BottomR *= KG2LB;
			Internal.BalanceBoard.Lb.Total	 *= KG2LB;
			}
			break;

		case MOTION_PLUS:
			{
			bMotionPlusDetected = true;
			bMotionPlusEnabled  = true;

			short yaw   = ((unsigned short)buff[offset+3] & 0xFC)<<6 |
						   (unsigned short)buff[offset+0];
			short pitch = ((unsigned short)buff[offset+5] & 0xFC)<<6 |
                           (unsigned short)buff[offset+2];
			short roll  = ((unsigned short)buff[offset+4] & 0xFC)<<6 |
				           (unsigned short)buff[offset+1];

			// we get one set of bogus values when the MotionPlus is disconnected,
			//  so ignore them
			if((yaw != 0x3fff) || (pitch != 0x3fff) || (roll != 0x3fff))
				{
				wiimote_state::motion_plus::sensors_raw &raw = Internal.MotionPlus.Raw;
	
				if((raw.Yaw != yaw) || (raw.Pitch != pitch) || (raw.Roll  != roll))
					changed |= MOTIONPLUS_SPEED_CHANGED;

				raw.Yaw   = yaw;
				raw.Pitch = pitch;
				raw.Roll  = roll;
	
				// convert to float values
				bool    yaw_slow = (buff[offset+3] & 0x2) == 0x2;
				bool  pitch_slow = (buff[offset+3] & 0x1) == 0x1;
				bool   roll_slow = (buff[offset+4] & 0x2) == 0x2;
				float y_scale    =   yaw_slow? 0.05f : 0.25f;
				float p_scale    = pitch_slow? 0.05f : 0.25f;
				float r_scale    =  roll_slow? 0.05f : 0.25f;
			
				Internal.MotionPlus.Speed.Yaw   = -(raw.Yaw   - 0x1F7F) * y_scale;
		        Internal.MotionPlus.Speed.Pitch = -(raw.Pitch - 0x1F7F) * p_scale;
			    Internal.MotionPlus.Speed.Roll  = -(raw.Roll  - 0x1F7F) * r_scale;
	
				// show if there's an extension plugged into the MotionPlus:
				bool extension = buff[offset+4] & 1;
				if(extension != bMotionPlusExtension)
					{
					if(extension) {
						TRACE(_T(".. MotionPlus extension found."));
						changed |= MOTIONPLUS_EXTENSION_CONNECTED;
						}
					else{
						TRACE(_T(".. MotionPlus' extension disconnected."));
						changed |= MOTIONPLUS_EXTENSION_DISCONNECTED;
						}
					}
				bMotionPlusExtension = extension;
				}
			// while we're getting data, the plus is obviously detected/enabled
//			bMotionPlusDetected = bMotionPlusEnabled = true;
			}
			break;
		}
	
	return changed;
	}
// ------------------------------------------------------------------------------------
int wiimote::ParseReadAddress (BYTE* buff)
	{
	// decode the address that was queried:
	int address = buff[4]<<8 | buff[5];
	int size    = buff[3] >> 4;
        (void)size;
	int changed	= 0;

	if((buff[3] & 0x08) != 0) {
		WARN(_T("error: read address not valid."));
		_ASSERT(0);
		return NO_CHANGE;
		}
	// address read failed (write-only)?
	else if((buff[3] & 0x07) != 0)
		{
		// this also happens when attempting to detect a non-existant MotionPlus
		if(MotionPlusDetectCount)
			{
			--MotionPlusDetectCount;
			if(Internal.ExtensionType == MOTION_PLUS)
				{
				if(bMotionPlusDetected)
					TRACE(_T(".. MotionPlus removed."));
				bMotionPlusDetected  = false;
				bMotionPlusEnabled   = false;
				// the MotionPlus can sometimes get confused - initializing
				//  extenions fixes it:
//				if(address == 0xfa)
//					InitializeExtension();
				}
			}
		else
			WARN(_T("error: attempt to read from write-only register 0x%X."), buff[3]);

		return NO_CHANGE;
		}

	// *NOTE*: this is a major (but convenient) hack!  The returned data only
	//          contains the lower two bytes of the address that was queried.
	//			as these don't collide between any of the addresses/registers
	//			we currently read, it's OK to match just those two bytes

	// skip the header
	buff += 6;

	switch(address)
		{
		case (REGISTER_CALIBRATION & 0xffff):
			{
			_ASSERT(size == 6);
			TRACE(_T(".. got wiimote calibration."));
			Internal.CalibrationInfo.X0 = buff[0];
			Internal.CalibrationInfo.Y0 = buff[1];
			Internal.CalibrationInfo.Z0 = buff[2];
			Internal.CalibrationInfo.XG = buff[4];
			Internal.CalibrationInfo.YG = buff[5];
			Internal.CalibrationInfo.ZG = buff[6];
			//changed |= CALIBRATION_CHANGED;	
			}
			break;
			
		// note: this covers both the normal extension and motion plus extension
		//        addresses (0x4a400fa / 0x4a600fa)
		case (REGISTER_EXTENSION_TYPE & 0xffff):
			{
			_ASSERT(size == 5);
			QWORD type = *(QWORD*)buff;

//			TRACE(_T("(found extension 0x%I64x)"), type);

			static const QWORD NUNCHUK		       = 0x000020A40000ULL;
			static const QWORD CLASSIC		       = 0x010120A40000ULL;
			static const QWORD GH3_GHWT_GUITAR     = 0x030120A40000ULL;
			static const QWORD GHWT_DRUMS	       = 0x030120A40001ULL;
			static const QWORD BALANCE_BOARD	   = 0x020420A40000ULL;
			static const QWORD MOTION_PLUS		   = 0x050420A40000ULL;
			static const QWORD MOTION_PLUS_DETECT  = 0x050020a60000ULL;
			static const QWORD MOTION_PLUS_DETECT2 = 0x050420a60000ULL;
			static const QWORD PARTIALLY_INSERTED  = 0xffffffffffffULL;

			// MotionPlus: _before_ it's been activated
			if((type == MOTION_PLUS_DETECT) || (type == MOTION_PLUS_DETECT2))
				{
				if(!bMotionPlusDetected) {
					TRACE(_T("Motion Plus detected!"));
					changed |= MOTIONPLUS_DETECTED;
					}
				bMotionPlusDetected = true;
				--MotionPlusDetectCount;
				break;
				}

			#define IF_TYPE(id, ...) /* sometimes it comes in more than once */ \
			    if(type == id)                                      \
			    {                                                   \
                    if(Internal.ExtensionType == wiimote_state::id) \
                        break;                                      \
                    Internal.ExtensionType = wiimote_state::id;     \
                    __VA_ARGS__;                                    \
                }

			// MotionPlus: once it's activated & mapped to the standard ext. port
			IF_TYPE(MOTION_PLUS,
				TRACE(_T(".. Motion Plus!"));
				// and start a query for the calibration data
				ReadAddress(REGISTER_EXTENSION_CALIBRATION, 16);
				bMotionPlusDetected = true;
            )
			else IF_TYPE(NUNCHUK,
				TRACE(_T(".. Nunchuk!"));
				bMotionPlusEnabled = false;
				// and start a query for the calibration data
				ReadAddress(REGISTER_EXTENSION_CALIBRATION, 16);
            )
			else IF_TYPE(CLASSIC,
				TRACE(_T(".. Classic Controller!"));
				bMotionPlusEnabled = false;
				// and start a query for the calibration data
				ReadAddress(REGISTER_EXTENSION_CALIBRATION, 16);
            )
			else IF_TYPE(GH3_GHWT_GUITAR,
				// sometimes it comes in more than once?
				TRACE(_T(".. GH3/GHWT Guitar Controller!"));
				bMotionPlusEnabled = false;
				// and start a query for the calibration data
				ReadAddress(REGISTER_EXTENSION_CALIBRATION, 16);
            )
			else IF_TYPE(GHWT_DRUMS,
				TRACE(_T(".. GHWT Drums!"));
				bMotionPlusEnabled = false;
				// and start a query for the calibration data
				ReadAddress(REGISTER_EXTENSION_CALIBRATION, 16);
            )
			else IF_TYPE(BALANCE_BOARD,
				TRACE(_T(".. Balance Board!"));
				bMotionPlusEnabled = false;
				// and start a query for the calibration data
				ReadAddress(REGISTER_BALANCE_CALIBRATION, 24);
            )
			else if(type == PARTIALLY_INSERTED) {
				// sometimes it comes in more than once?
				if(Internal.ExtensionType == wiimote_state::PARTIALLY_INSERTED)
					Sleep(50);
				TRACE(_T(".. partially inserted!"));
				bMotionPlusEnabled = false;
				Internal.ExtensionType = wiimote_state::PARTIALLY_INSERTED;
				changed |= EXTENSION_PARTIALLY_INSERTED;
				// try initializing the extension again by requesting another
				//  status report (this usually fixes it)
				Internal.bExtension = false;
				RequestStatusReport();
			}
			else
				TRACE(_T("unknown extension controller found (0x%I64x)"), type);
			}
			break;

		case (REGISTER_EXTENSION_CALIBRATION & 0xffff):
		case (REGISTER_BALANCE_CALIBRATION   & 0xffff):
			{
//			_ASSERT(((Internal.ExtensionType == BALANCE_BOARD) && (size == 31)) ||
//					((Internal.ExtensionType != BALANCE_BOARD) && (size == 15)));

			switch(Internal.ExtensionType)
				{
				case wiimote_state::NUNCHUK:
					{
					wiimote_state::nunchuk::calibration_info
						&calib = Internal.Nunchuk.CalibrationInfo;

					calib.X0   = buff[ 0];
					calib.Y0   = buff[ 1];
					calib.Z0   = buff[ 2];
					calib.XG   = buff[ 4];
					calib.YG   = buff[ 5];
					calib.ZG   = buff[ 6];
					calib.MaxX = buff[ 8];
					calib.MinX = buff[ 9];
					calib.MidX = buff[10];
					calib.MaxY = buff[11];
					calib.MinY = buff[12];
					calib.MidY = buff[13];

					changed |= NUNCHUK_CONNECTED;//|NUNCHUK_CALIBRATION_CHANGED;
					// reenable reports
//					SetReportType(ReportType);
					}
					break;
				
				case wiimote_state::CLASSIC:
				case wiimote_state::GH3_GHWT_GUITAR:
				case wiimote_state::GHWT_DRUMS:
					{
					wiimote_state::classic_controller::calibration_info
						&calib = Internal.ClassicController.CalibrationInfo;
					
					calib.MaxXL = buff[ 0] >> 2;
					calib.MinXL = buff[ 1] >> 2;
					calib.MidXL = buff[ 2] >> 2;
					calib.MaxYL = buff[ 3] >> 2;
					calib.MinYL = buff[ 4] >> 2;
					calib.MidYL = buff[ 5] >> 2;
					calib.MaxXR = buff[ 6] >> 3;
					calib.MinXR = buff[ 7] >> 3;
					calib.MidXR = buff[ 8] >> 3;
					calib.MaxYR = buff[ 9] >> 3;
					calib.MinYR = buff[10] >> 3;
					calib.MidYR = buff[11] >> 3;
					// this doesn't seem right...
					//	calib.MinTriggerL = buff[12] >> 3;
					//	calib.MaxTriggerL = buff[14] >> 3;
					//	calib.MinTriggerR = buff[13] >> 3;
					//	calib.MaxTriggerR = buff[15] >> 3;
					calib.MinTriggerL = 0;
					calib.MaxTriggerL = 31;
					calib.MinTriggerR = 0;
					calib.MaxTriggerR = 31;

					changed |= CLASSIC_CONNECTED;//|CLASSIC_CALIBRATION_CHANGED;
					// reenable reports
//					SetReportType(ReportType);
					}
					break;

				case BALANCE_BOARD:
					{
					// first part, 0 & 17kg calibration values
					wiimote_state::balance_board::calibration_info
						&calib = Internal.BalanceBoard.CalibrationInfo;

					calib.Kg0 .TopR	   = (short)((short)buff[0] << 8 | buff[1]);
					calib.Kg0 .BottomR = (short)((short)buff[2] << 8 | buff[3]);
					calib.Kg0 .TopL	   = (short)((short)buff[4] << 8 | buff[5]);
					calib.Kg0 .BottomL = (short)((short)buff[6] << 8 | buff[7]);

					calib.Kg17.TopR	   = (short)((short)buff[8] << 8 | buff[9]);
					calib.Kg17.BottomR = (short)((short)buff[10] << 8 | buff[11]);
					calib.Kg17.TopL	   = (short)((short)buff[12] << 8 | buff[13]);
					calib.Kg17.BottomL = (short)((short)buff[14] << 8 | buff[15]);

					// 2nd part is scanned above
					}
					break;

				case MOTION_PLUS:
					{
					// TODO: not known how the calibration values work
					changed |= MOTIONPLUS_ENABLED;
					bMotionPlusEnabled = true;
					bInitInProgress	   = false;
					// reenable reports
//					SetReportType(ReportType);
					}
					break;
				}
			case 0x34:
				{
				if(Internal.ExtensionType == BALANCE_BOARD)
					{
					wiimote_state::balance_board::calibration_info
						&calib = Internal.BalanceBoard.CalibrationInfo;

					// 2nd part of the balance board calibration,
					//  34kg calibration values
					calib.Kg34.TopR    = (short)((short)buff[0] << 8 | buff[1]);
					calib.Kg34.BottomR = (short)((short)buff[2] << 8 | buff[3]);
					calib.Kg34.TopL    = (short)((short)buff[4] << 8 | buff[5]);
					calib.Kg34.BottomL = (short)((short)buff[6] << 8 | buff[7]);
						
					changed |= BALANCE_CONNECTED;
					// reenable reports
					SetReportType(IN_BUTTONS_BALANCE_BOARD);
					}
				// else unknown what these are for
				}
			bInitInProgress = false;
			}
			break;

		default:
//			_ASSERT(0); // shouldn't happen
			break;
		}
	
	return changed;
	}
// ------------------------------------------------------------------------------------
void wiimote::ReadCalibration ()
	{
	TRACE(_T("Requestion wiimote calibration:"));
	// this appears to change the report type to 0x31
	ReadAddress(REGISTER_CALIBRATION, 7);
	}
// ------------------------------------------------------------------------------------
void wiimote::EnableIR (wiimote_state::ir::mode mode)
	{
	Internal.IR.Mode = mode;

	BYTE buff [REPORT_LENGTH] = {0};
	buff[0] = OUT_IR;
	buff[1] = 0x04 | GetRumbleBit();
	WriteReport(buff);

	memset(buff, 0, REPORT_LENGTH);
	buff[0] = OUT_IR2;
	buff[1] = 0x04 | GetRumbleBit();
	WriteReport(buff);

	static const BYTE ir_sens1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x00,
									0xc0};
	static const BYTE ir_sens2[] = {0x40, 0x00};

	WriteData(REGISTER_IR, 0x08);
	Sleep(25); 	// wait a little to make IR more reliable (for some)
	WriteData(REGISTER_IR_SENSITIVITY_1, sizeof(ir_sens1), ir_sens1);
	WriteData(REGISTER_IR_SENSITIVITY_2, sizeof(ir_sens2), ir_sens2);
	WriteData(REGISTER_IR_MODE, (BYTE)mode);
	}
// ------------------------------------------------------------------------------------
void wiimote::DisableIR ()
	{
	Internal.IR.Mode = wiimote_state::ir::OFF;

	BYTE buff [REPORT_LENGTH] = {0};
	buff[0] = OUT_IR;
	buff[1] = GetRumbleBit();
	WriteReport(buff);

	memset(buff, 0, REPORT_LENGTH);
	buff[0] = OUT_IR2;
	buff[1] = GetRumbleBit();
	WriteReport(buff);
	}
// ------------------------------------------------------------------------------------
unsigned __stdcall wiimote::HIDwriteThreadfunc (void* param)
	{
	_ASSERT(param);
	TRACE(_T("(starting HID write thread)"));
	wiimote &remote = *(wiimote*)param;

	while(remote.Handle != INVALID_HANDLE_VALUE)
		{
		// try to write the oldest entry in the queue
#ifdef USE_DYNAMIC_HIDQUEUE
		if(!remote.HIDwriteQueue.empty())
#else
		if(!remote.HID.IsEmpty())
#endif
			{
#ifdef BEEP_DEBUG_WRITES
			Beep(1500,1);
#endif
			EnterCriticalSection(&remote.HIDwriteQueueLock);
#ifdef USE_DYNAMIC_HIDQUEUE
			 BYTE *buff = remote.HIDwriteQueue.front();
			 _ASSERT(buff);
#else
			 BYTE *buff = remote.HID.Queue[remote.HID.ReadIndex].Report;
#endif
			LeaveCriticalSection(&remote.HIDwriteQueueLock);

			if(!HidD_SetOutputReport(remote.Handle, buff, REPORT_LENGTH))
				{
				DWORD err = GetLastError();
if(err==ERROR_BUSY)
TRACE(_T("**** HID WRITE: BUSY ****"));
else if(err == ERROR_NOT_READY)
TRACE(_T("**** HID WRITE: NOT READY ****"));

				if((err != ERROR_BUSY)	&&	 // "the requested resource is in use"
				   (err != ERROR_NOT_READY)) // "the device is not ready"
					{
					if(err == ERROR_NOT_SUPPORTED) {
						WARN(_T("BT Stack doesn't suport HID writes!"));
						goto remove_entry;
						}
					else{
						DEEP_TRACE(_T("HID write failed (err %u)! - "), err);
						// if this worked previously, the connection was probably lost
						if(remote.IsConnected())
							remote.bConnectionLost = true;
						}
					//_T("aborting write thread"), err);
					//return 911;
					}
				}
			else{
remove_entry:
				EnterCriticalSection(&remote.HIDwriteQueueLock);
#ifdef USE_DYNAMIC_HIDQUEUE
				 remote.HIDwriteQueue.pop();
				 delete[] buff;
#else
				 remote.HID.ReadIndex++;
				 remote.HID.ReadIndex &= (hid::MAX_QUEUE_ENTRIES-1);
#endif
				LeaveCriticalSection(&remote.HIDwriteQueueLock);
				}
			}
		Sleep(1);
		}

	TRACE(_T("ending HID write thread"));
	return 0;
	}
// ------------------------------------------------------------------------------------
bool wiimote::WriteReport (BYTE *buff)
	{
#ifdef BEEP_DEBUG_WRITES
	Beep(2000,1);
#endif

#ifdef _DEBUG
	#define DEEP_TRACE_TYPE(type)	case OUT_##type: DEEP_TRACE(_T("WriteReport: ")\
																_T(#type)); break
	switch(buff[0])
		{
		DEEP_TRACE_TYPE(NONE);
		DEEP_TRACE_TYPE(LEDs);
		DEEP_TRACE_TYPE(TYPE);
		DEEP_TRACE_TYPE(IR);
		DEEP_TRACE_TYPE(SPEAKER_ENABLE);
		DEEP_TRACE_TYPE(STATUS);
		DEEP_TRACE_TYPE(WRITEMEMORY);
		DEEP_TRACE_TYPE(READMEMORY);
		DEEP_TRACE_TYPE(SPEAKER_DATA);
		DEEP_TRACE_TYPE(SPEAKER_MUTE);
		DEEP_TRACE_TYPE(IR2);
		default:
			TRACE(_T("WriteReport: type [%02x][%02x]"), buff[1], buff[2]);
		}
#endif

	if(bUseHIDwrite)
		{
		/* no where to release handle, I have to do it myself */
		if (HIDwriteThread)
		{
			if (WaitForSingleObject(HIDwriteThread, 0) == WAIT_OBJECT_0) {
				CloseHandle(HIDwriteThread);
				HIDwriteThread = NULL;
			}
		}
		// HidD_SetOutputReport: +: works on MS Bluetooth stacks (WriteFile doesn't).
		//						 -: is synchronous, so make it async
		if(!HIDwriteThread)
			{
			HIDwriteThread = (HANDLE)_beginthreadex(NULL, 0, HIDwriteThreadfunc,
													this, 0, NULL);
			_ASSERT(HIDwriteThread);
			if(!HIDwriteThread) {
				WARN(_T("couldn't create HID write thread!"));
				return false;
				}
			SetThreadPriority(HIDwriteThread, WORKER_THREAD_PRIORITY);
			}

		// insert the write request into the thread's queue
#ifdef USE_DYNAMIC_HIDQUEUE
		EnterCriticalSection(&HIDwriteQueueLock);
		 BYTE *buff_copy = new BYTE[REPORT_LENGTH];
#else
		// allocate the HID write queue once
		if(!HID.Queue && !HID.Allocate())
			return false;

		EnterCriticalSection(&HIDwriteQueueLock);
		 BYTE *buff_copy = HID.Queue[HID.WriteIndex].Report;
#endif
		 memcpy(buff_copy, buff, REPORT_LENGTH);

#ifdef USE_DYNAMIC_HIDQUEUE
		 HIDwriteQueue.push(buff_copy);
#else
		 HID.WriteIndex++;
		 HID.WriteIndex &= (HID.MAX_QUEUE_ENTRIES-1);

		 // check if the fixed report queue has overflown:
		 //  if this ASSERT triggers, the HID write queue (that stores reports
		 //   for asynchronous output by HIDwriteThreadfunc) has overflown.
		 //  this can happen if the connection with the wiimote has been lost
		 //   and in that case is harmless.
		 //
		 //  if it happens during normal operation though you need to increase
		 //   hid::MAX_QUEUE_ENTRIES to the next power-of-2 (see comments)
		 //   _and_ email me the working setting so I can update the next release
		 _ASSERT(HID.WriteIndex != HID.ReadIndex);
#endif
		LeaveCriticalSection(&HIDwriteQueueLock);
		return true;
		}

	// WriteFile:
	DWORD written;
	if(!WriteFile(Handle, buff, REPORT_LENGTH, &written, &Overlapped))
		{
		DWORD error = GetLastError();
		if(error != ERROR_IO_PENDING) {
			TRACE(_T("WriteFile failed, err: %u!"), error);
			// if it worked previously, assume we lost the connection
			if(IsConnected())
				bConnectionLost = true;
#ifndef USE_DYNAMIC_HIDQUEUE
			HID.Deallocate();
#endif
			return false;
			}
		}
	return true;
	}
// ------------------------------------------------------------------------------------
// experimental speaker support:
// ------------------------------------------------------------------------------------
bool wiimote::MuteSpeaker (bool on)
	{
	_ASSERT(IsConnected());
	if(!IsConnected())
		return false;

	if(Internal.Speaker.bMuted == on)
		return true;

	if(on) TRACE(_T("muting speaker."  ));
	else   TRACE(_T("unmuting speaker."));

	BYTE buff [REPORT_LENGTH] = {0};
	buff[0] = OUT_SPEAKER_MUTE;
	buff[1] = (on? 0x04 : 0x00) | GetRumbleBit();
	if(!WriteReport(buff))
		return false;
	Sleep(1);
	Internal.Speaker.bMuted = on;
	return true;
	}
// ------------------------------------------------------------------------------------
bool wiimote::EnableSpeaker (bool on)
	{
	_ASSERT(IsConnected());
	if(!IsConnected())
		return false;

	if(Internal.Speaker.bEnabled == on)
		return true;

	if(on) TRACE(_T("enabling speaker.")); else TRACE(_T("disabling speaker."));

	BYTE buff [REPORT_LENGTH] = {0};
	buff[0] = OUT_SPEAKER_ENABLE;
	buff[1] = (on? 0x04 : 0x00) | GetRumbleBit();
	if(!WriteReport(buff))
		return false;

	if(!on) {
		Internal.Speaker.Freq   = FREQ_NONE;
		Internal.Speaker.Volume = 0;
		MuteSpeaker(true);
		}

	Internal.Speaker.bEnabled = on;
	return true;
	}
// ------------------------------------------------------------------------------------
#ifdef TR4 // TEMP, ignore
 extern int hzinc;
#endif
// ------------------------------------------------------------------------------------
unsigned __stdcall wiimote::SampleStreamThreadfunc (void* param)
	{
	TRACE(_T("(starting sample thread)"));
	// sends a simple square wave sample stream
	wiimote &remote = *(wiimote*)param;
	
	static BYTE squarewave_report[REPORT_LENGTH] =
		{ OUT_SPEAKER_DATA, 20<<3, 0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,
								   0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3, };
	static BYTE sample_report [REPORT_LENGTH] = 
		{ OUT_SPEAKER_DATA, 0 };

	bool		   last_playing    = false;
	DWORD		   frame		   = 0;
	DWORD		   frame_start     = 0;
	unsigned	   total_samples   = 0;
	unsigned	   sample_index	   = 0;
	wiimote_sample *current_sample = NULL;
	
	// TODO: duration!!
	while(remote.IsConnected())
		{
		bool playing = remote.IsPlayingAudio();
		
		if(!playing)
			Sleep(1);
		else{
			const unsigned freq_hz  = FreqLookup[remote.Internal.Speaker.Freq];
#ifdef TR4
			const float    frame_ms = 1000 / ((freq_hz+hzinc) / 40.f); // 20bytes = 40 samples per write
#else
			const float    frame_ms = 1000 / (freq_hz		  / 40.f); // 20bytes = 40 samples per write
#endif

			// has the sample just changed?
			bool sample_changed = (current_sample != remote.CurrentSample);
			current_sample		= (wiimote_sample*)remote.CurrentSample;

// (attempts to minimise glitches, doesn't seem to help though)
//#define FIRSTFRAME_IS_SILENT	// send all-zero for first frame

#ifdef FIRSTFRAME_IS_SILENT
			bool silent_frame = false;
#endif
			if(!last_playing || sample_changed) {
				frame		  = 0;
				frame_start   = timeGetTime();
				total_samples = current_sample? current_sample->length : 0;
				sample_index  = 0;
#ifdef FIRSTFRAME_IS_SILENT
				silent_frame  = true;
#endif
				}

			// are we streaming a sample?
			if(current_sample)
				{
				if(sample_index < current_sample->length)
					{
					// (remember that samples are 4bit, ie. 2 per byte)
					unsigned samples_left   = (current_sample->length - sample_index);
					unsigned report_samples = std::min(samples_left, (unsigned)40);
					// round the entries up to the nearest multiple of 2
					unsigned report_entries = (report_samples+1) >> 1;
					
					sample_report[1] = (BYTE)((report_entries<<3) |
											  remote.GetRumbleBit());
#ifdef FIRSTFRAME_IS_SILENT
					if(silent_frame) {
						// send all-zeroes
						for(unsigned index=0; index<report_entries; index++)
							sample_report[2+index] = 0;
						remote.WriteReport(sample_report);
						}
					else
#endif
					{
						for(unsigned index=0; index<report_entries; index++)
							sample_report[2+index] =
									current_sample->samples[(sample_index>>1)+index];
						remote.WriteReport(sample_report);
						sample_index += report_samples;
						}
					}
				else{
					// we reached the sample end
					remote.CurrentSample		   = NULL;
					current_sample				   = NULL;
					remote.Internal.Speaker.Freq   = FREQ_NONE;
					remote.Internal.Speaker.Volume = 0;
					}
				}
			// no, a squarewave
			else{
				squarewave_report[1] = (20<<3) | remote.GetRumbleBit();
				remote.WriteReport(squarewave_report);
#if 0
				// verify that we're sending at the correct rate (we are)
				DWORD elapsed		   = (timeGetTime()-frame_start);
				unsigned total_samples = frame * 40;
				float elapsed_secs	   = elapsed / 1000.f;
				float sent_persec	   = total_samples / elapsed_secs;
#endif
				}

			frame++;

			// send the first two buffers immediately? (attempts to lessen startup
			//  startup glitches by assuming we're filling a small sample
			//  (or general input) buffer on the wiimote) - doesn't seem to help
//			if(frame > 2) {
				while((timeGetTime()-frame_start) < (unsigned)(frame*frame_ms))
					Sleep(1);
//				}
			}

		last_playing = playing;
		}
	
	TRACE(_T("(ending sample thread)"));
	return 0;
	}
// ------------------------------------------------------------------------------------
bool wiimote::Load16bitMonoSampleWAV (const TCHAR* filepath, wiimote_sample &out)
	{
	// converts unsigned 16bit mono .wav audio data to the 4bit ADPCM variant
	//  used by the Wiimote (at least the closest match so far), and returns
	//  the data in a BYTE array (caller must delete[] it when no longer needed):
	memset(&out, 0, sizeof(out));

	TRACE(_T("Loading '%s'"), filepath);

	FILE *file;
#if (_MSC_VER >= 1400) // VC 2005+
	_tfopen_s(&file, filepath, _T("rb"));
#else
	file = _tfopen(filepath, _T("rb"));
#endif
	_ASSERT(file);
	if(!file) {
		WARN(_T("Couldn't open '%s"), filepath);
		return false;
		}

	// parse the .wav file
	struct riff_chunkheader {
		char  ckID [4];
		DWORD ckSize;
		char  formType [4];
		};
	struct chunk_header {
		char  ckID [4];
		DWORD ckSize;
		};
	union {
		WAVEFORMATEX		 x;
		WAVEFORMATEXTENSIBLE xe;
		} wf = {0};

	riff_chunkheader riff_chunkheader;
	chunk_header	 chunk_header;
	speaker_freq	 freq = FREQ_NONE;

	#define READ(data)			if(fread(&data, sizeof(data), 1, file) != 1) { \
									TRACE(_T(".wav file corrupt"));			   \
									fclose(file);							   \
									return false;							   \
									}
	#define READ_SIZE(ptr,size)	if(fread(ptr, size, 1, file) != 1) {		   \
									TRACE(_T(".wav file corrupt"));			   \
									fclose(file);							   \
									return false;							   \
									}
	// read the riff chunk header
	READ(riff_chunkheader);

	// valid RIFF file?
	_ASSERT(!strncmp(riff_chunkheader.ckID, "RIFF", 4));
	if(strncmp(riff_chunkheader.ckID, "RIFF", 4))
		goto unsupported; // nope
	// valid WAV variant?
	_ASSERT(!strncmp(riff_chunkheader.formType, "WAVE", 4));
	if(strncmp(riff_chunkheader.formType, "WAVE", 4))
		goto unsupported; // nope

	// find the format & data chunks
	while(1)
		{
		READ(chunk_header);
		
		if(!strncmp(chunk_header.ckID, "fmt ", 4))
			{
			// not a valid .wav file?
			if(chunk_header.ckSize < 16 ||
			   chunk_header.ckSize > sizeof(WAVEFORMATEXTENSIBLE))
				goto unsupported;

			READ_SIZE((BYTE*)&wf.x, chunk_header.ckSize);

			// now we know it's true wav file
			bool extensible = (wf.x.wFormatTag == WAVE_FORMAT_EXTENSIBLE);
			int format	    = extensible? wf.xe.SubFormat.Data1 :
										  wf.x .wFormatTag;
			// must be uncompressed PCM (the format comparisons also work on
			//  the 'extensible' header, even though they're named differently)
			if(format != WAVE_FORMAT_PCM) {
				TRACE(_T(".. not uncompressed PCM"));
				goto unsupported;
				}

			// must be mono, 16bit
			if((wf.x.nChannels != 1) || (wf.x.wBitsPerSample != 16)) {
				TRACE(_T(".. %d bit, %d channel%s"), wf.x.wBitsPerSample,
													 wf.x.nChannels,
													(wf.x.nChannels>1? _T("s"):_T("")));
				goto unsupported;
				}

			// must be _near_ a supported speaker frequency range (but allow some
			//  tolerance, especially as the speaker freq values aren't final yet):
			unsigned	   sample_freq = wf.x.nSamplesPerSec;
			const unsigned epsilon	   = 100; // for now
			
			for(unsigned index=1; index<std::size(FreqLookup); index++)
				{
				if((sample_freq+epsilon) >= FreqLookup[index] &&
				   (sample_freq-epsilon) <= FreqLookup[index]) {
					freq = (speaker_freq)index;
					TRACE(_T(".. using speaker freq %u"), FreqLookup[index]);
					break;
					}
				}
			if(freq == FREQ_NONE) {
				WARN(_T("Couldn't (loosely) match .wav samplerate %u Hz to speaker"),
					 sample_freq);
				goto unsupported;
				}
			}
		else if(!strncmp(chunk_header.ckID, "data", 4))
			{
			// make sure we got a valid fmt chunk first
			if(!wf.x.nBlockAlign)
				goto corrupt_file;

			// grab the data
			unsigned total_samples = chunk_header.ckSize / wf.x.nBlockAlign;
			if(total_samples == 0)
				goto corrupt_file;
			
			short *samples = new short[total_samples];
			size_t read = fread(samples, 2, total_samples, file);
			fclose(file);
			if(read != total_samples)
				{
				if(read == 0) {
					delete[] samples;
					goto corrupt_file;
					}
				// got a different number, but use them anyway
				WARN(_T("found %s .wav audio data than expected (%u/%u samples)"),
					((read < total_samples)? _T("less") : _T("more")),
					read, total_samples);

				total_samples = read;
				}

			// and convert them
			bool res = Convert16bitMonoSamples(samples, true, total_samples, freq,
											   out);
			delete[] samples;
			return res;
			}
		else{
			// unknown chunk, skip its data
			DWORD chunk_bytes = (chunk_header.ckSize + 1) & ~1L;
			if(fseek(file, chunk_bytes, SEEK_CUR))
				goto corrupt_file;
			}
		}

corrupt_file:
	WARN(_T(".wav file is corrupt"));
	fclose(file);
	return false;

unsupported:
	WARN(_T(".wav file format not supported (must be mono 16bit PCM)"));
	fclose(file);
	return false;
	}
// ------------------------------------------------------------------------------------
bool wiimote::Load16BitMonoSampleRAW (const TCHAR*   filepath,
									  bool		     _signed,
									  speaker_freq   freq,
									  wiimote_sample &out)
	{
	// converts (.wav style) unsigned 16bit mono raw data to the 4bit ADPCM variant
	//  used by the Wiimote, and returns the data in a BYTE array (caller must
	//  delete[] it when no longer needed):
	memset(&out, 0, sizeof(out));

	// get the length of the file
	struct _stat file_info;
	if(_tstat(filepath, &file_info)) {
		WARN(_T("couldn't get filesize for '%s'"), filepath);
		return false;
		}
	
	DWORD len = file_info.st_size;
	_ASSERT(len);
	if(!len) {
		WARN(_T("zero-size sample file '%s'"), filepath);
		return false;
		}

	unsigned total_samples = (len+1) / 2; // round up just in case file is corrupt
	// allocate a buffer to hold the samples to convert
	short *samples = new short[total_samples]; 
	_ASSERT(samples);
	if(!samples) {
		TRACE(_T("Couldn't open '%s"), filepath);
		return false;
		}

	// load them
	FILE *file;
	bool res;
#if (_MSC_VER >= 1400) // VC 2005+
	_tfopen_s(&file, filepath, _T("rb"));
#else
	file = _tfopen(filepath, _T("rb"));
#endif
	_ASSERT(file);
	if(!file) {
		TRACE(_T("Couldn't open '%s"), filepath);
        goto error;
        }

	res = (fread(samples, 1, len, file) == len);
	fclose(file);
	if(!res) {
		WARN(_T("Couldn't load file '%s'"), filepath);
		goto error;
		}

	// and convert them
	res = Convert16bitMonoSamples(samples, _signed, total_samples, freq, out);
	delete[] samples;
	return res;

error:
	delete[] samples;
	return false;
	}
// ------------------------------------------------------------------------------------
bool wiimote::Convert16bitMonoSamples (const short*   samples,
									   bool		      _signed,
									   DWORD		  length,
									   speaker_freq   freq,
									   wiimote_sample &out)
	{
	// converts 16bit mono sample data to the native 4bit format used by the Wiimote,
	//  and returns the data in a BYTE array (caller must delete[] when no
	//  longer needed):
	memset(&out, 0, sizeof(0));

	_ASSERT(samples && length);
	if(!samples || !length)
		return false;

	// allocate the output buffer
	out.samples = new BYTE[length];
	_ASSERT(out.samples);
	if(!out.samples)
		return false;

	// clear it
	memset(out.samples, 0, length);
	out.length = length;
	out.freq   = freq;

	// ADPCM code, adapted from
	//  http://www.wiindows.org/index.php/Talk:Wiimote#Input.2FOutput_Reports
	static const int index_table[16] = {  -1,  -1,  -1,  -1,   2,   4,   6,   8,
										  -1,  -1,  -1,  -1,   2,   4,   6,   8 };
	static const int diff_table [16] = {   1,   3,   5,   7,   9,  11,  13,  15,
										  -1,  -3,  -5,  -7,  -9, -11, -13,  15 };
	static const int step_scale [16] = { 230, 230, 230, 230, 307, 409, 512, 614,
										 230, 230, 230, 230, 307, 409, 512, 614 };
	// Encode to ADPCM, on initialization set adpcm_prev_value to 0 and adpcm_step
	//  to 127 (these variables must be preserved across reports)
	int adpcm_prev_value = 0;
	int adpcm_step		 = 127;

	for(size_t i=0; i<length; i++)
		{
		// convert to 16bit signed
		int value = samples[i];// (8bit) << 8);// | samples[i]; // dither it?
		if(!_signed)
			value -= 32768;
		// encode:
		int  diff = value - adpcm_prev_value;
		BYTE encoded_val = 0;
		if(diff < 0) {
			encoded_val |= 8;
			diff = -diff;
			}
		diff = (diff << 2) / adpcm_step;
		if (diff > 7)
			diff = 7;
		encoded_val |= diff;
		adpcm_prev_value += ((adpcm_step * diff_table[encoded_val]) / 8);
		if(adpcm_prev_value  >  0x7fff)
			adpcm_prev_value =  0x7fff;
		if(adpcm_prev_value  < -0x8000)
			adpcm_prev_value = -0x8000;
		adpcm_step = (adpcm_step * step_scale[encoded_val]) >> 8;
		if(adpcm_step < 127)
			adpcm_step = 127;
		if(adpcm_step > 24567)
			adpcm_step = 24567;
		if(i & 1)
			out.samples[i>>1] |= encoded_val;
		else
			out.samples[i>>1] |= encoded_val << 4;
		}

	return true;
	}
// ------------------------------------------------------------------------------------
bool wiimote::PlaySample (const wiimote_sample &sample, BYTE volume, 
						  speaker_freq freq_override)
	{
	_ASSERT(IsConnected());
	if(!IsConnected())
		return false;

	speaker_freq freq = freq_override? freq_override : sample.freq;

	TRACE(_T("playing sample."));
	EnableSpeaker(true);
	MuteSpeaker  (true);

#if 0
	// combine everything into one write - faster, seems to work?
	BYTE bytes[9] = { 0x00, 0x00, 0x00, 10+freq, vol, 0x00, 0x00, 0x01, 0x01 };
	WriteData(0x04a20001, sizeof(bytes), bytes);
#else
	// Write 0x01 to register 0x04a20009 
	WriteData(0x04a20009, 0x01);
	// Write 0x08 to register 0x04a20001 
	WriteData(0x04a20001, 0x08);
	// Write 7-byte configuration to registers 0x04a20001-0x04a20008 
	BYTE bytes[7] = { '\0', '\0', '\0', BYTE(10 + freq), volume, '\0', '\0' };
	WriteData(0x04a20001, sizeof(bytes), bytes);
	// + Write 0x01 to register 0x04a20008 
	WriteData(0x04a20008, 0x01);
#endif

	Internal.Speaker.Freq   = freq;
	Internal.Speaker.Volume = volume;
	CurrentSample			= &sample;

	MuteSpeaker(false);

	return StartSampleThread();
	}
// ------------------------------------------------------------------------------------
bool wiimote::StartSampleThread ()
	{
	if(SampleThread)
		return true;

	SampleThread = (HANDLE)_beginthreadex(NULL, 0, SampleStreamThreadfunc,
										  this, 0, NULL);
	_ASSERT(SampleThread);
	if(!SampleThread) {
		WARN(_T("couldn't create sample thread!"));
		MuteSpeaker  (true);
		EnableSpeaker(false);	
		return false;
		}
	SetThreadPriority(SampleThread, WORKER_THREAD_PRIORITY);
	return true;
	}
// ------------------------------------------------------------------------------------
bool wiimote::PlaySquareWave (speaker_freq freq, BYTE volume)
	{
	_ASSERT(IsConnected());
	if(!IsConnected())
		return false;

	// if we're already playing a sample, stop it first
	if(IsPlayingSample())
		CurrentSample = NULL;
	// if we're already playing a square wave at this freq and volume, return
	else if(IsPlayingAudio() && (Internal.Speaker.Freq   == freq) &&
								(Internal.Speaker.Volume == volume))
		return true;

	TRACE(_T("playing square wave."));
	// stop playing samples
	CurrentSample = 0;

	EnableSpeaker(true);
	MuteSpeaker  (true);

#if 0
	// combined everything into one write - much faster, seems to work?
	BYTE bytes[9] = { 0x00, 0x00, 0x00, freq, volume, 0x00, 0x00, 0x01, 0x1 };
	WriteData(0x04a20001, sizeof(bytes), bytes);
#else
	// write 0x01 to register 0xa20009 
	WriteData(0x04a20009, 0x01);
	// write 0x08 to register 0xa20001 
	WriteData(0x04a20001, 0x08);
	// write default sound mode (4bit ADPCM, we assume) 7-byte configuration
	//  to registers 0xa20001-0xa20008 
	BYTE bytes[7] = { '\0', '\0', '\0', BYTE(10+freq), volume, '\0', '\0' };
	WriteData(0x04a20001, sizeof(bytes), bytes);
	// write 0x01 to register 0xa20008 
	WriteData(0x04a20008, 0x01);
#endif

	Internal.Speaker.Freq   = freq;
	Internal.Speaker.Volume = volume;

	MuteSpeaker(false);
	return StartSampleThread();
	}
// ------------------------------------------------------------------------------------
void wiimote::RecordState (state_history	  &events_out,
						   unsigned			  max_time_ms,
						   state_change_flags change_trigger)
	{
	// user being naughty?
	if(Recording.bEnabled)
		StopRecording();

	// clear the list
	if(!events_out.empty())
		events_out.clear();

	// start recording 
	Recording.StateHistory = &events_out;
	Recording.StartTimeMS  = timeGetTime();
	Recording.EndTimeMS    = Recording.StartTimeMS + max_time_ms;
	Recording.TriggerFlags = change_trigger;
	// as this call happens outside the read/parse thread, set the boolean
	//  which will enable reocrding last, so that all params are in place.
	// TODO: * stricly speaking this only works on VC2005+ or better, as it
	//		   automatically places a memory barrier on volatile variables - earlier/
	//         other compilers may reorder the assignments!). *
	Recording.bEnabled	   = true;
	}
// ------------------------------------------------------------------------------------
void wiimote::StopRecording ()
	{
	if(!Recording.bEnabled)
		return;

	Recording.bEnabled = false;
	// make sure the read/parse thread has time to notice the change (else it might
	//  still write one more state to the list)
	Sleep(10); // too much?
	}
// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------
