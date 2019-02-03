/* Homepage         http://facetracknoir.sourceforge.net/home/default.htm        *
 *                                                                               *
 * ISC License (ISC)                                                             *
 *                                                                               *
 * Copyright (c) 2015, Wim Vriend                                                *
 * Copyright (c) 2014, 2017, 2019 Stanislaw Halik                                *
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
        sxs_manifest(b, "simconnect-manifest", 0)
    {}
};

class simconnect : private QThread, public IProtocol
{
    Q_OBJECT
public:
    simconnect() = default;
    ~simconnect() override;
    module_status initialize() override;
    void pose(const double* headpose) override;
    QString game_name() override;
    void run() override;

private:
    void handler();

    enum {
        SIMCONNECT_RECV_ID_EXCEPTION = 2,
        SIMCONNECT_RECV_ID_QUIT = 3,
        SIMCONNECT_RECV_ID_EVENT_FRAME = 7,
    };

    struct SIMCONNECT_RECV
    {
        DWORD   dwSize;
        DWORD   dwVersion;
        DWORD   dwID;
    };

    typedef void (CALLBACK *DispatchProc)(SIMCONNECT_RECV*, DWORD, void*);

    typedef HRESULT (WINAPI *importSimConnect_Open)(HANDLE * phSimConnect, LPCSTR szName, HWND hWnd, DWORD UserEventWin32, HANDLE hEventHandle, DWORD ConfigIndex);
    typedef HRESULT (WINAPI *importSimConnect_Close)(HANDLE hSimConnect);
    typedef HRESULT (WINAPI *importSimConnect_CameraSetRelative6DOF)(HANDLE hSimConnect, float fDeltaX, float fDeltaY, float fDeltaZ, float fPitchDeg, float fBankDeg, float fHeadingDeg);
    typedef HRESULT (WINAPI *importSimConnect_CallDispatch)(HANDLE hSimConnect, DispatchProc pfcnDispatch, void * pContext);
    typedef HRESULT (WINAPI *importSimConnect_SubscribeToSystemEvent)(HANDLE hSimConnect, DWORD EventID, const char * SystemEventName);

    std::atomic<float> virtSCPosX {0};
    std::atomic<float> virtSCPosY {0};
    std::atomic<float> virtSCPosZ {0};
    std::atomic<float> virtSCRotX {0};
    std::atomic<float> virtSCRotY {0};
    std::atomic<float> virtSCRotZ {0};

    importSimConnect_Open simconnect_open = nullptr;
    importSimConnect_Close simconnect_close = nullptr;
    importSimConnect_CameraSetRelative6DOF simconnect_set6DOF = nullptr;
    importSimConnect_CallDispatch simconnect_calldispatch = nullptr;
    importSimConnect_SubscribeToSystemEvent simconnect_subscribetosystemevent = nullptr;

    HANDLE handle = nullptr;
    std::atomic<bool> reconnect = false;
    static void CALLBACK processNextSimconnectEvent(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext);
    settings s;
    QLibrary library;
};

class simconnect_ui: public IProtocolDialog
{
    Q_OBJECT

    Ui::UICSCControls ui;
    settings s;

public:
    simconnect_ui();
    void register_protocol(IProtocol *) override {}
    void unregister_protocol() override {}

private slots:
    void doOK();
    void doCancel();
};

class simconnect_metadata : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("Microsoft FSX SimConnect"); }
    QIcon icon() override { return QIcon(":/images/fsx.png"); }
};
