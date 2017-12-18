/* Homepage         http://facetracknoir.sourceforge.net/home/default.htm        *
 *                                                                               *
 * ISC License (ISC)                                                             *
 *                                                                               *
 * Copyright (c) 2015, Wim Vriend                                                *
 * Copyright (c) 2014, 2017 Stanislaw Halik                                      *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */
#pragma once
#include "api/plugin-api.hpp"
#include "ui_ftnoir_sccontrols.h"

#include <atomic>

#include <QThread>
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include "options/options.hpp"
using namespace options;
#include <windows.h>

struct settings : opts {
    value<int> sxs_manifest;
    settings() :
        opts("proto-simconnect"),
        sxs_manifest(b, "simconnect-manifest", 2)
    {}
};

class simconnect : public IProtocol, private QThread
{
public:
    simconnect();
    ~simconnect() override;
    module_status initialize() override;
    void pose(const double* headpose);
    void handle();
    QString game_name() {
        return otr_tr("FS2004/FSX");
    }
private:
    enum {
        SIMCONNECT_RECV_ID_NULL,
        SIMCONNECT_RECV_ID_EXCEPTION = 2,
        SIMCONNECT_RECV_ID_QUIT = 3,
        SIMCONNECT_RECV_ID_EVENT_FRAME = 7,
    };

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

    std::atomic<float> virtSCPosX;
    std::atomic<float> virtSCPosY;
    std::atomic<float> virtSCPosZ;
    std::atomic<float> virtSCRotX;
    std::atomic<float> virtSCRotY;
    std::atomic<float> virtSCRotZ;

    importSimConnect_Open simconnect_open;
    importSimConnect_Close simconnect_close;
    importSimConnect_CameraSetRelative6DOF simconnect_set6DOF;
    importSimConnect_CallDispatch simconnect_calldispatch;
    importSimConnect_SubscribeToSystemEvent simconnect_subscribetosystemevent;

    HANDLE hSimConnect;
    std::atomic<bool> should_reconnect;
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

class simconnectDll : public Metadata
{
public:
    QString name() { return otr_tr("Microsoft FSX SimConnect"); }
    QIcon icon() { return QIcon(":/images/fsx.png"); }
};
