/********************************************************************************
* FaceTrackNoIR         This program is a private project of some enthusiastic          *
*                                       gamers from Holland, who don't like to pay much for                     *
*                                       head-tracking.                                                                                          *
*                                                                                                                                                               *
* Copyright (C) 2010-2011       Wim Vriend (Developing)                                                         *
*                                                       Ron Hendriks (Researching and Testing)                          *
*                                                                                                                                                               *
* Homepage                                                                                                                                              *
*                                                                                                                                                               *
* This program is free software; you can redistribute it and/or modify it               *
* under the terms of the GNU General Public License as published by the                 *
* Free Software Foundation; either version 3 of the License, or (at your                *
* option) any later version.                                                                                                    *
*                                                                                                                                                               *
* This program is distributed in the hope that it will be useful, but                   *
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY    *
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for   *
* more details.                                                                                                                                 *
*                                                                                                                                                               *
* You should have received a copy of the GNU General Public License along               *
* with this program; if not, see <http://www.gnu.org/licenses/>.                                *
*                                                                                                                                                               *
* SCServer              SCServer is the Class, that communicates headpose-data                  *
*                               to games, using the SimConnect.dll.                                                     *
*                               SimConnect.dll is a so called 'side-by-side' assembly, so it    *
*                               must be treated as such...                                                                              *
********************************************************************************/
#pragma once
#include "opentrack/plugin-api.hpp"

#include "ui_ftnoir_sccontrols.h"
#include <QThread>
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include "opentrack/options.hpp"
using namespace options;
#include <windows.h>

struct settings : opts {
    value<int> sxs_manifest;
    settings() :
        opts("proto-simconnect"),
        sxs_manifest(b, "sxs-manifest-version", 0)
    {}
};

class FTNoIR_Protocol : public IProtocol, private QThread
{
public:
    FTNoIR_Protocol();
    ~FTNoIR_Protocol() override;
    bool correct();
    void pose(const double* headpose);
    void handle();
    QString game_name() {
        return "FS2004/FSX";
    }
private:
    enum { SIMCONNECT_RECV_ID_EVENT_FRAME = 7 };

    #pragma pack(push, 1)
    struct SIMCONNECT_RECV
    {
        DWORD   dwSize;
        DWORD   dwVersion;
        DWORD   dwID;
    };
    #pragma pack(pop)

    typedef void (CALLBACK *DispatchProc)(SIMCONNECT_RECV*, DWORD, void*);

    typedef HRESULT (WINAPI *importSimConnect_Open)(HANDLE * phSimConnect, LPCSTR szName, HWND hWnd, DWORD UserEventWin32, HANDLE hEventHandle, DWORD ConfigIndex);
    typedef HRESULT (WINAPI *importSimConnect_Close)(HANDLE hSimConnect);
    typedef HRESULT (WINAPI *importSimConnect_CameraSetRelative6DOF)(HANDLE hSimConnect, float fDeltaX, float fDeltaY, float fDeltaZ, float fPitchDeg, float fBankDeg, float fHeadingDeg);
    typedef HRESULT (WINAPI *importSimConnect_CallDispatch)(HANDLE hSimConnect, DispatchProc pfcnDispatch, void * pContext);
    typedef HRESULT (WINAPI *importSimConnect_SubscribeToSystemEvent)(HANDLE hSimConnect, DWORD EventID, const char * SystemEventName);

    void run() override;
    volatile bool should_stop;

    volatile float virtSCPosX;
    volatile float virtSCPosY;
    volatile float virtSCPosZ;
    volatile float virtSCRotX;
    volatile float virtSCRotY;
    volatile float virtSCRotZ;

    importSimConnect_Open simconnect_open;
    importSimConnect_Close simconnect_close;
    importSimConnect_CameraSetRelative6DOF simconnect_set6DOF;
    importSimConnect_CallDispatch simconnect_calldispatch;
    importSimConnect_SubscribeToSystemEvent simconnect_subscribetosystemevent;

    HANDLE hSimConnect;
    static void CALLBACK processNextSimconnectEvent(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext);
    settings s;
    QLibrary SCClientLib;
};

class SCControls: public IProtocolDialog
{
    Q_OBJECT
public:
    SCControls();
    void register_protocol(IProtocol *) {}
    void unregister_protocol() {}
private:
    Ui::UICSCControls ui;
    settings s;
private slots:
    void doOK();
    void doCancel();
};

class FTNoIR_ProtocolDll : public Metadata
{
public:
    QString name() { return QString("Microsoft FSX SimConnect"); }
    QIcon icon() { return QIcon(":/images/fsx.png"); }
};
