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
#include <QMutex>
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
    void pose(const double* headpose, const double*) override;
    QString game_name() override;
    void run() override;

private:
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

    typedef void (CALLBACK *SC_DispatchProc)(SIMCONNECT_RECV*, DWORD, void*);

    typedef HRESULT (WINAPI *SC_Open)(HANDLE* phSimConnect, LPCSTR szName, HWND hWnd, DWORD UserEventWin32, HANDLE hEventHandle, DWORD ConfigIndex);
    typedef HRESULT (WINAPI *SC_Close)(HANDLE hSimConnect);
    typedef HRESULT (WINAPI *SC_CameraSetRelative6DOF)(HANDLE hSimConnect, float fDeltaX, float fDeltaY, float fDeltaZ, float fPitchDeg, float fBankDeg, float fHeadingDeg);
    typedef HRESULT (WINAPI *SC_CallDispatch)(HANDLE hSimConnect, SC_DispatchProc pfcnDispatch, void * pContext);
    typedef HRESULT (WINAPI *SC_SubscribeToSystemEvent)(HANDLE hSimConnect, DWORD EventID, const char * SystemEventName);

    float data[6] {};
    QMutex mtx;

    SC_Open                    simconnect_open         = nullptr;
    SC_Close                   simconnect_close        = nullptr;
    SC_CameraSetRelative6DOF   simconnect_set6DOF      = nullptr;
    SC_CallDispatch            simconnect_calldispatch = nullptr;
    SC_SubscribeToSystemEvent  simconnect_subscribe    = nullptr;

    HANDLE handle = nullptr;
    std::atomic<bool> reconnect = false;
    static void CALLBACK event_handler(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext);
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
