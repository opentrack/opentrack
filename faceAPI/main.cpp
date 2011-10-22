/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2011	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*********************************************************************************/
/*
	Modifications (last one on top):
		20110501 - WVR: Added some command to be handled from FaceTrackNoIR (settings dialog).
		20110322 - WVR: Somehow the video-widget of faceAPI version 3.2.6. does not
					    work with FaceTrackNoIR (Qt issue?!). To be able to use 
						release 3.2.6 of faceAPI anyway, this console-app is used.
						It exchanges data with FaceTrackNoIR via shared-memory...
*/

//Precompiled header
#include "stdafx.h"

//FaceAPI headers
#include "sm_api.h"
#include "ftnoir_tracker_sm_types.h"
#include "utils.h"

//local headers
#include "build_options.h"

//namespaces
using namespace std;
using namespace sm::faceapi::samplecode;

//
// global variables
//
HANDLE					hSMMemMap = NULL;
SMMemMap				*pMemData;
HANDLE					hSMMutex;
smEngineHeadPoseData	new_head_pose;
bool					stopCommand = false;
bool					ftnoirConnected = false;

//enums
enum GROUP_ID
{
    GROUP0=0,
};

enum EVENT_ID
{
	EVENT_PING=0,
	EVENT_INIT,
};

enum INPUT_ID
{
    INPUT0=0,
};

//function definitions
void	updateHeadPose(smEngineHeadPoseData* temp_head_pose);
bool	SMCreateMapping();

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//FaceAPI function implementations
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void STDCALL receiveLogMessage(void *, const char *buf, int /*buf_len*/)
{
    Lock lock(g_mutex); // serialize logging calls from different threads to avoid garbled output.
    //cout << string(buf);
}

// Callback function for face-data
void STDCALL receiveFaceData(void *, smEngineFaceData face_data, smCameraVideoFrame video_frame)
{   
    Lock lock(g_mutex);

    // Get info including data pointer to original image from camera
    smImageInfo video_frame_image_info;
    THROW_ON_ERROR(smImageGetInfo(video_frame.image_handle, &video_frame_image_info)); // reentrant, so ok

    // video_frame_image_info.plane_addr[*] now point to the image memory planes. 
    // The memory is only valid until the end of this routine unless you call smImageAddRef(video_frame.image_handle).
    // So you can deep copy the image data here, or use smImageAddRef() and just copy the pointer.
    // If you use smImageAddRef() you are responsible for calling smImageDestroy() to avoid a memory leak later.

    // In this callback you will typically want to copy the smEngineFaceData data into your own data-structure.
    // Since the smEngineFaceData contains multiple pod types copying it is not atomic and 
    // a mutex is required to avoid the race-condition with any thread simultaneously 
    // reading from your data-structure.
    // Such a race condition will not crash your code but will create weird noise in the tracking data.

    if (g_do_face_data_printing)
    {
        //cout << video_frame << " " << face_data;

        // Save any face texture to a PNG file
        if (face_data.texture)
        {
            // Create a unique filename
            std::stringstream filename;
            filename << "face_" << video_frame.frame_num << ".png";        
            // Try saving to a file
            if (saveToPNGFile(filename.str(), face_data.texture->image_info) == SM_API_OK)
            {
                cout << "Saved face-texture to " << filename.str() << std::endl;
            }
            else
            {
                cout << "Error saving face-texture to " << filename.str() << std::endl;
            }
        }
    }
}

// Callback function for head-pose
void STDCALL receiveHeadPose(void *,smEngineHeadPoseData head_pose, smCameraVideoFrame video_frame)
{
    Lock lock(g_mutex);

    // Get info including data pointer to original image from camera
    smImageInfo video_frame_image_info;
    THROW_ON_ERROR(smImageGetInfo(video_frame.image_handle, &video_frame_image_info)); // reentrant, so ok

    // video_frame_image_info.plane_addr[*] now point to the image memory planes. 
    // The memory is only valid until the end of this routine unless you call smImageAddRef(video_frame.image_handle).
    // So you can deep copy the image data here, or use smImageAddRef() and just copy the pointer.
    // If you use smImageAddRef() you are responsible for calling smImageDestroy() to avoid a memory leak later.

    // In this callback you will typically want to copy the smEngineFaceData data into your own data-structure.
    // Since the smEngineFaceData contains multiple pod types copying it is not atomic and 
    // a mutex is required to avoid the race-condition with any thread simultaneously 
    // reading from your data-structure.
    // Such a race condition will not crash your code but will create weird noise in the tracking data.

    if (g_do_head_pose_printing)
    {
        //cout << video_frame << " " << head_pose << std::endl;
    }

	//make a copy of the new head pose data and send it to simconnect
	//when we get a simmconnect frame event the new offset will be applied to the camera
	updateHeadPose(&head_pose);
}

// Create the first available camera detected on the system, and return its handle
smCameraHandle createFirstCamera()
{
    // Detect cameras
    smCameraInfoList info_list;
    THROW_ON_ERROR(smCameraCreateInfoList(&info_list));

    if (info_list.num_cameras == 0)
    {
        throw runtime_error("No cameras were detected");
    }
    else
    {
        cout << "The followings cameras were detected: " << endl;
        for (int i=0; i<info_list.num_cameras; ++i)
        {
            char buf[1024];
            cout << "    " << i << ". Type: " << info_list.info[i].type;
            THROW_ON_ERROR(smStringWriteBuffer(info_list.info[i].model,buf,1024));
            cout << " Model: " << string(buf);
            cout << " Instance: " << info_list.info[i].instance_index << endl;
            // Print all the possible formats for the camera
            for (int j=0; j<info_list.info[i].num_formats; j++)
            {
                smCameraVideoFormat video_format = info_list.info[i].formats[j];
                cout << "     - Format: ";
                cout << " res (" << video_format.res.w << "," << video_format.res.h << ")";
                cout << " image code " << video_format.format;
                cout << " framerate " << video_format.framerate << "(hz)";
                cout << " upside-down? " << (video_format.is_upside_down ? "y":"n") << endl;
            }
        }
    }

    // Create the first camera detected on the system
    smCameraHandle camera_handle = 0;
    THROW_ON_ERROR(smCameraCreate(&info_list.info[0],   // Use first camera
                                  0,                    // Use default settings for lens
                                  &camera_handle));

    // Destroy the info list
    smCameraDestroyInfoList(&info_list);

    return camera_handle;
}

// The main function: setup a tracking engine and show a video window, then loop on the keyboard.
void run()
{
	char msg[100];
	int state;

	// Capture control-C
//    signal(SIGINT, CtrlCHandler);

    // Make the console window a bit bigger (see utils.h)
    initConsole();

	#ifdef _DEBUG
    // Log API debugging information to a file
    THROW_ON_ERROR(smLoggingSetFileOutputEnable(SM_API_TRUE));

    // Hook up log message callback
    THROW_ON_ERROR(smLoggingRegisterCallback(0,receiveLogMessage));
	#endif

    // Get the version
    int major, minor, maint;
    THROW_ON_ERROR(smAPIVersion(&major, &minor, &maint));
    cout << endl << "API VERSION: " << major << "." << minor << "." << maint << "." << endl << endl;
    // Print detailed license info
    char *buff;
    int size;
    THROW_ON_ERROR(smAPILicenseInfoString(0,&size,SM_API_TRUE));
    buff = new char[size];
    THROW_ON_ERROR(smAPILicenseInfoString(buff,&size,SM_API_TRUE));
    cout << "LICENSE: " << buff << endl << endl;
    // Determine if non-commercial restrictions apply
    const bool non_commercial_license = smAPINonCommercialLicense() == SM_API_TRUE;

    // Initialize the API
    THROW_ON_ERROR(smAPIInit());

	#ifdef _DEBUG
    // Get the path to the logfile
    smStringHandle logfile_path_handle = 0;
    THROW_ON_ERROR(smStringCreate(&logfile_path_handle));
    THROW_ON_ERROR(smLoggingGetPath(logfile_path_handle));
    int buf_len = 0;
    unsigned short *buf = 0;
    THROW_ON_ERROR(smStringGetBufferW(logfile_path_handle,(wchar_t **)&buf,&buf_len));
    wcout << "Writing log to file: " << wstring((wchar_t *)buf) << endl;
    THROW_ON_ERROR(smStringDestroy(&logfile_path_handle));
	#endif

    // Register the WDM category of cameras
    THROW_ON_ERROR(smCameraRegisterType(SM_API_CAMERA_TYPE_WDM));

    smEngineHandle engine_handle = 0;
    smCameraHandle camera_handle = 0;
    if (non_commercial_license)
    {
        // Create a new Head-Tracker engine that uses the camera
        THROW_ON_ERROR(smEngineCreate(SM_API_ENGINE_LATEST_HEAD_TRACKER,&engine_handle));
    }
    else
    {
        // Print out a list of connected cameras, and choose the first camera on the system
        camera_handle = createFirstCamera();
        // Create a new Head-Tracker engine that uses the camera
        THROW_ON_ERROR(smEngineCreateWithCamera(SM_API_ENGINE_LATEST_HEAD_TRACKER,camera_handle,&engine_handle));
    }

    // Check license for particular engine version (always ok for non-commercial license)
    const bool engine_licensed = smEngineIsLicensed(engine_handle) == SM_API_OK;

    cout << "-----------------------------------------------------" << endl;
    cout << "Press 'r' to restart tracking" << endl;
    cout << "Press 'a' to toggle auto-restart mode" << endl;
    if (!non_commercial_license)
    {
        cout << "Press 'l' to toggle lip-tracking" << endl;
        cout << "Press 'e' to toggle eyebrow-tracking" << endl;
    }
    if (engine_licensed)
    {
        cout << "Press 'h' to toggle printing of head-pose data" << endl;
        cout << "Press 'f' to toggle printing of face-landmark data" << endl;
    }
    cout << "Press '1' to toggle face coordinate frame axes" << endl;
    cout << "Press '2' to toggle performance info" << endl;
    cout << "Press '3' to toggle face mask" << endl;
    cout << "Press '4' to toggle face landmarks" << endl;
    cout << "Press CTRL-C or 'q' to quit" << endl;
    cout << "-----------------------------------------------------" << endl;

    // Hook up callbacks to receive output data from engine.
    // These functions will return errors if the engine is not licensed.
    if (engine_licensed)
    {
		#if (USE_HEADPOSE_CALLBACK==1)
		#pragma message("Using Headpose Callback")
        THROW_ON_ERROR(smHTRegisterHeadPoseCallback(engine_handle,0,receiveHeadPose));
		#endif
        if (!non_commercial_license)
        {
            THROW_ON_ERROR(smHTRegisterFaceDataCallback(engine_handle,0,receiveFaceData));
        }
    }
    else
    {
        cout << "Engine is not licensed, cannot obtain any output data." << endl;
    }

    if (!non_commercial_license)
    {
        // Enable lip and eyebrow tracking
        THROW_ON_ERROR(smHTSetLipTrackingEnabled(engine_handle,SM_API_TRUE));
        THROW_ON_ERROR(smHTSetEyebrowTrackingEnabled(engine_handle,SM_API_TRUE));
    }

    // Create and show a video-display window
	// Set the initial filter-level, from the INI-file
	smVideoDisplayHandle video_display_handle = 0;
	if (pMemData) {
		THROW_ON_ERROR(smVideoDisplayCreate(engine_handle,&video_display_handle,(smWindowHandle) pMemData->handle,TRUE));
		THROW_ON_ERROR(smHTV2SetHeadPoseFilterLevel(engine_handle, pMemData->initial_filter_level));
		pMemData->handshake = 0;
	}
	else {
		THROW_ON_ERROR(smVideoDisplayCreate(engine_handle,&video_display_handle,0,TRUE));
	}

    // Setup the VideoDisplay
    THROW_ON_ERROR(smVideoDisplaySetFlags(video_display_handle,g_overlay_flags));

    // Get the handle to the window and change the title to "Hello World"
    smWindowHandle win_handle = 0;
    THROW_ON_ERROR(smVideoDisplayGetWindowHandle(video_display_handle,&win_handle));    
    SetWindowText(win_handle, _T("faceAPI Video-widget"));
	MoveWindow(win_handle, 0, 0, 250, 180, true);

    // Loop on the keyboard
    while (processKeyPress(engine_handle, video_display_handle) && !stopCommand)
    {
        // Read and print the current head-pose (if not using the callback mechanism)
		#if (USE_HEADPOSE_CALLBACK==0)
		#pragma message("Polling Headpose Manually")
		if (engine_licensed)
		{
			smEngineHeadPoseData head_pose;
			Lock lock(g_mutex);
			
			THROW_ON_ERROR(smHTCurrentHeadPose(engine_handle,&head_pose));
			if (g_do_head_pose_printing)
			{
				std::cout << head_pose << std::endl;
			}

		}
		#endif

        // NOTE: If you have a windows event loop in your program you 
        // will not need to call smAPIProcessEvents(). This manually redraws the video window.
        THROW_ON_ERROR(smAPIProcessEvents());

        // Prevent CPU overload in our simple loop.
        const int frame_period_ms = 10;
        Sleep(frame_period_ms); 

		//
		// Process the command sent by FaceTrackNoIR.
		//
		if (ftnoirConnected && (pMemData != 0)) {

			sprintf_s(msg, "Command: %d, \n", pMemData->command, pMemData->par_val_int);
			OutputDebugStringA(msg);
			std::cout << msg;

			//
			//
			// Determine the trackers' state and send it to FaceTrackNoIR.
			//
			THROW_ON_ERROR(smEngineGetState(engine_handle, &state));
			pMemData->state = state;
			pMemData->handshake += 1;

			//
			// Check if FaceTrackNoIR is still 'in contact'.
			// FaceTrackNoIR will reset the handshake, every time in writes data.
			// If the value rises too high, this exe will stop itself...
			//
			if ( pMemData->handshake > 200) {
				stopCommand = TRUE;
			}

			//
			// Check if a command was issued and do something with it!
			//
			switch (pMemData->command) {
				case FT_SM_START:

					//
					// Only execute Start, if the engine is not yet tracking.
					//
					if (state != SM_API_ENGINE_STATE_HT_TRACKING) {
						THROW_ON_ERROR(smEngineStart(engine_handle));	// Start tracking
					}
					pMemData->command = 0;								// Reset
					break;

				case FT_SM_STOP:
					THROW_ON_ERROR(smEngineStop(engine_handle));		// Stop tracking
					pMemData->command = 0;								// Reset
					break;

				case FT_SM_EXIT:
					THROW_ON_ERROR(smEngineStop(engine_handle));		// Stop tracking
					stopCommand = TRUE;
					pMemData->command = 0;								// Reset
					break;

				case FT_SM_SET_PAR_FILTER:
					THROW_ON_ERROR(smHTV2SetHeadPoseFilterLevel(engine_handle, pMemData->par_val_int));
					pMemData->command = 0;								// Reset
					break;

				case FT_SM_SHOW_CAM:
					THROW_ON_ERROR(smEngineShowCameraControlPanel(engine_handle));
					pMemData->command = 0;								// Reset
					break;

				default:
					pMemData->command = 0;								// Reset
					// should never be reached
				break;
			}
		}
	}			// While(1)

    // Destroy engine
    THROW_ON_ERROR(smEngineDestroy(&engine_handle));
    // Destroy video display
    THROW_ON_ERROR(smVideoDisplayDestroy(&video_display_handle));

	if (ftnoirConnected) {
		if ( pMemData != NULL ) {
			UnmapViewOfFile ( pMemData );
		}
		
		if (hSMMutex != 0) {
			CloseHandle( hSMMutex );
		}
		hSMMutex = 0;
		
		if (hSMMemMap != 0) {
			CloseHandle( hSMMemMap );
		}
		hSMMemMap = 0;
	}

} // run()

// Application entry point
int _tmain(int /*argc*/, _TCHAR** /*argv*/)
{
	OutputDebugString(_T("_tmain() says: Starting Function\n"));

	try
    {
		if (SMCreateMapping()) {
			run();
		}
    }
    catch (exception &e)
    {
        cerr << e.what() << endl;
    }

    return smAPIQuit();
}

//
// This is called exactly once for each FaceAPI callback and must be within an exclusive lock
//
void updateHeadPose(smEngineHeadPoseData* temp_head_pose)
{
	//
	// Check if the pointer is OK and wait for the Mutex.
	//
	if ( (pMemData != NULL) && (WaitForSingleObject(hSMMutex, 100) == WAIT_OBJECT_0) ) {
		
		//
		// Copy the Raw measurements directly to the client.
		//
		if (temp_head_pose->confidence > 0.0f)
		{
			memcpy(&pMemData->data.new_pose,temp_head_pose,sizeof(smEngineHeadPoseData));
		}
		ReleaseMutex(hSMMutex);
	}
};

//
// Create a memory-mapping to the faceAPI data.
// It contains the tracking data, a command-code from FaceTrackNoIR
//
//
bool SMCreateMapping()
{
	OutputDebugString(_T("FTCreateMapping says: Starting Function\n"));

	//
	// A FileMapping is used to create 'shared memory' between the faceAPI and FaceTrackNoIR.
	// FaceTrackNoIR creates the mapping, this program only opens it.
	// If it's not there: the program was apparently started by the user instead of FaceTrackNoIR...
	//
	// Open an existing FileMapping, Read/Write access
	//
	hSMMemMap = OpenFileMappingA( FILE_MAP_ALL_ACCESS , false , (LPCSTR) SM_MM_DATA );
	if ( ( hSMMemMap != 0 ) ) {
		ftnoirConnected = true;
		OutputDebugString(_T("FTCreateMapping says: FileMapping opened successfully...\n"));
		pMemData = (SMMemMap *) MapViewOfFile(hSMMemMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TFaceData));
		if (pMemData != NULL) {
			OutputDebugString(_T("FTCreateMapping says: MapViewOfFile OK.\n"));
			pMemData->state = 0;
		}
	    hSMMutex = CreateMutexA(NULL, false, SM_MUTEX);
	}
	else {
		OutputDebugString(_T("FTCreateMapping says: FileMapping not opened...FaceTrackNoIR not connected!\n"));
		ftnoirConnected = false;
		pMemData = 0;
	}

	return true;
}
