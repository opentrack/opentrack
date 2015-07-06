/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010-2011	Wim Vriend (Developing)								*
*							Ron Hendriks (Researching and Testing)				*
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
*																				*
* FTNoIR_Protocol: the Class, that communicates headpose-data					*
*				to games, using the SimConnect.dll.		         				*
*				SimConnect.dll is a so called 'side-by-side' assembly, so it	*
*				must be treated as such...										*
********************************************************************************/
#include "ftnoir_protocol_sc.h"
#include "opentrack/plugin-api.hpp"
#include "opentrack/thread.hpp"

FTNoIR_Protocol::FTNoIR_Protocol() : should_stop(false), hSimConnect(nullptr)
{
}

FTNoIR_Protocol::~FTNoIR_Protocol()
{
    should_stop = true;
    wait();
}

void FTNoIR_Protocol::run()
{
    Affinity thr;

    (void) timeBeginPeriod(1);

    while (!should_stop)
    {
        if (SUCCEEDED(simconnect_open(&hSimConnect, "opentrack", NULL, 0, 0, 0)))
        {
            simconnect_subscribetosystemevent(hSimConnect, 0, "Frame");

            while (!should_stop)
            {
                if (FAILED(simconnect_calldispatch(hSimConnect, processNextSimconnectEvent, reinterpret_cast<void*>(this))))
                    break;
                Sleep(3);
            }

            (void) simconnect_close(hSimConnect);
        }

        Sleep(100);
    }

    (void) timeEndPeriod(1);
}

void FTNoIR_Protocol::pose( const double *headpose ) {
    virtSCRotX = -headpose[Pitch];					// degrees
    virtSCRotY = headpose[Yaw];
    virtSCRotZ = headpose[Roll];

    virtSCPosX = headpose[TX]/100.f;						// cm to meters
    virtSCPosY = headpose[TY]/100.f;
    virtSCPosZ = -headpose[TZ]/100.f;
}

#pragma GCC diagnostic ignored "-Wmissing-field-initializers"

class ActivationContext {
public:
    ActivationContext(const int resid) : ok(false) {
        hactctx = INVALID_HANDLE_VALUE;
        actctx_cookie = 0;
        ACTCTXA actx = {0};
        actx.cbSize = sizeof(ACTCTXA);
        actx.lpResourceName = MAKEINTRESOURCEA(resid);
        actx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
#ifdef _MSC_VER
#	define PREFIX ""
#else
#	define PREFIX "lib"
#endif 
        QString path = QCoreApplication::applicationDirPath() + "/" PREFIX "opentrack-proto-simconnect.dll";
        QByteArray name = QFile::encodeName(path);
        actx.lpSource = name.constData();
        hactctx = CreateActCtxA(&actx);
        actctx_cookie = 0;
        if (hactctx != INVALID_HANDLE_VALUE) {
            if (!ActivateActCtx(hactctx, &actctx_cookie)) {
                qDebug() << "SC: can't set win32 activation context" << GetLastError();
                ReleaseActCtx(hactctx);
                hactctx = INVALID_HANDLE_VALUE;
            }
            else
                ok = true;
        } else {
            qDebug() << "SC: can't create win32 activation context" << GetLastError();
        }
    }
    ~ActivationContext() {
        if (hactctx != INVALID_HANDLE_VALUE)
        {
            DeactivateActCtx(0, actctx_cookie);
            ReleaseActCtx(hactctx);
        }
    }
    bool is_ok() { return ok; }
private:
    ULONG_PTR actctx_cookie;
    HANDLE hactctx;
    bool ok;
};

bool FTNoIR_Protocol::correct()
{   
    if (!SCClientLib.isLoaded())                           
    {
        ActivationContext ctx(142 + static_cast<int>(s.sxs_manifest));
        
        if (ctx.is_ok())
        {
            SCClientLib.setFileName("SimConnect.dll");
            if (!SCClientLib.load()) {
                qDebug() << "SC load" << SCClientLib.errorString();
                return false;
            }
        }
        else
            return false;
    }

	//
	// Get the functions from the DLL.
	//
	simconnect_open = (importSimConnect_Open) SCClientLib.resolve("SimConnect_Open");
	if (simconnect_open == NULL) {
		qDebug() << "FTNoIR_Protocol::correct() says: SimConnect_Open function not found in DLL!";
		return false;
	}
	simconnect_set6DOF = (importSimConnect_CameraSetRelative6DOF) SCClientLib.resolve("SimConnect_CameraSetRelative6DOF");
	if (simconnect_set6DOF == NULL) {
		qDebug() << "FTNoIR_Protocol::correct() says: SimConnect_CameraSetRelative6DOF function not found in DLL!";
		return false;
	}
	simconnect_close = (importSimConnect_Close) SCClientLib.resolve("SimConnect_Close");
	if (simconnect_close == NULL) {
		qDebug() << "FTNoIR_Protocol::correct() says: SimConnect_Close function not found in DLL!";
		return false;
	}

	//return true;

	simconnect_calldispatch = (importSimConnect_CallDispatch) SCClientLib.resolve("SimConnect_CallDispatch");
	if (simconnect_calldispatch == NULL) {
		qDebug() << "FTNoIR_Protocol::correct() says: SimConnect_CallDispatch function not found in DLL!";
		return false;
	}

	simconnect_subscribetosystemevent = (importSimConnect_SubscribeToSystemEvent) SCClientLib.resolve("SimConnect_SubscribeToSystemEvent");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "FTNoIR_Protocol::correct() says: SimConnect_SubscribeToSystemEvent function not found in DLL!";
		return false;
	}

	simconnect_mapclienteventtosimevent = (importSimConnect_MapClientEventToSimEvent) SCClientLib.resolve("SimConnect_MapClientEventToSimEvent");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "FTNoIR_Protocol::correct() says: SimConnect_MapClientEventToSimEvent function not found in DLL!";
		return false;
	}

	simconnect_addclienteventtonotificationgroup = (importSimConnect_AddClientEventToNotificationGroup) SCClientLib.resolve("SimConnect_AddClientEventToNotificationGroup");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "FTNoIR_Protocol::correct() says: SimConnect_AddClientEventToNotificationGroup function not found in DLL!";
		return false;
	}

	simconnect_setnotificationgrouppriority = (importSimConnect_SetNotificationGroupPriority) SCClientLib.resolve("SimConnect_SetNotificationGroupPriority");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "FTNoIR_Protocol::correct() says: SimConnect_SetNotificationGroupPriority function not found in DLL!";
		return false;
	}

	qDebug() << "FTNoIR_Protocol::correct() says: SimConnect functions resolved in DLL!";
    
    start();

	return true;
}

void FTNoIR_Protocol::handle()
{
    if (prevSCPosX != virtSCPosX ||
        prevSCPosY != virtSCPosY ||
        prevSCPosZ != virtSCPosZ ||
        prevSCRotX != virtSCRotX ||
        prevSCRotY != virtSCRotY ||
        prevSCRotZ != virtSCRotZ)
    {
        (void) simconnect_set6DOF(hSimConnect, virtSCPosX, virtSCPosY, virtSCPosZ, virtSCRotX, virtSCRotZ, virtSCRotY);
    }
    
    prevSCPosX = virtSCPosX;
    prevSCPosY = virtSCPosY;
    prevSCPosZ = virtSCPosZ;
    prevSCRotX = virtSCRotX;
    prevSCRotY = virtSCRotY;
    prevSCRotZ = virtSCRotZ;
}

void CALLBACK FTNoIR_Protocol::processNextSimconnectEvent(SIMCONNECT_RECV* pData, DWORD, void *self_)
{
    FTNoIR_Protocol& self = *reinterpret_cast<FTNoIR_Protocol*>(self_);
    
    switch(pData->dwID)
    {
    default:
        break;
    case SIMCONNECT_RECV_ID_EVENT_FRAME:
        self.handle();
        break;
    }
}

extern "C" OPENTRACK_EXPORT IProtocol* GetConstructor()
{
    return new FTNoIR_Protocol;
}
