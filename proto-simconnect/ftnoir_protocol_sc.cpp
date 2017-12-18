/* Homepage         http://facetracknoir.sourceforge.net/home/default.htm        *
 *                                                                               *
 * ISC License (ISC)                                                             *
 *                                                                               *
 * Copyright (c) 2015, Wim Vriend
 * Copyright (c) 2014, Stanislaw Halik <sthalik@misaki.pl>
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */
#include "ftnoir_protocol_sc.h"
#include "api/plugin-api.hpp"
#include "opentrack-library-path.h"
#include "compat/timer.hpp"

simconnect::simconnect() : hSimConnect(nullptr), should_reconnect(false)
{
}

simconnect::~simconnect()
{
    requestInterruption();
    wait();
}

void simconnect::run()
{
    HANDLE event = CreateEventA(NULL, FALSE, FALSE, nullptr);

    if (event == nullptr)
    {
        qDebug() << "simconnect: event create" << GetLastError();
        return;
    }

    while (!isInterruptionRequested())
    {
        HRESULT hr;

        if (SUCCEEDED(hr = simconnect_open(&hSimConnect, "opentrack", nullptr, 0, event, 0)))
        {
            if (!SUCCEEDED(hr = simconnect_subscribetosystemevent(hSimConnect, 0, "Frame")))
            {
                qDebug() << "simconnect: can't subscribe to frame event:" << hr;
            }

            Timer tm;
            should_reconnect = false;

            if (SUCCEEDED(hr))
                while (!isInterruptionRequested())
                {
                    if (should_reconnect)
                        break;

                    if (WaitForSingleObject(event, 100) == WAIT_OBJECT_0)
                    {
                        tm.start();

                        if (!SUCCEEDED(hr = simconnect_calldispatch(hSimConnect, processNextSimconnectEvent, reinterpret_cast<void*>(this))))
                        {
                            qDebug() << "simconnect: calldispatch failed:" << hr;
                            break;
                        }
                    }
                    else
                    {
                        const int idle_seconds = tm.elapsed_seconds();

                        constexpr int max_idle_seconds = 2;

                        if (idle_seconds >= max_idle_seconds)
                        {
                            qDebug() << "simconnect: reconnect";
                            break;
                        }
                    }
                }

            (void) simconnect_close(hSimConnect);
        }
        else
            qDebug() << "simconnect: can't open handle:" << hr;

        if (!isInterruptionRequested())
            Sleep(3000);
    }

    qDebug() << "simconnect: exit";

    CloseHandle(event);
}

void simconnect::pose( const double *headpose )
{
    // euler degrees
    virtSCRotX = float(-headpose[Pitch]);
    virtSCRotY = float(headpose[Yaw]);
    virtSCRotZ = float(headpose[Roll]);

    // cm to meters
    virtSCPosX = float(-headpose[TX]/100);
    virtSCPosY = float(headpose[TY]/100);
    virtSCPosZ = float(-headpose[TZ]/100);
}

#ifdef __GNUC__
#   pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

class ActivationContext
{
public:
    ActivationContext(const int resid) :
        ok(false)
    {
        hactctx = INVALID_HANDLE_VALUE;
        actctx_cookie = 0;
        ACTCTXA actx = {};
        actx.cbSize = sizeof(actx);
        actx.lpResourceName = MAKEINTRESOURCEA(resid);
        actx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
#ifdef _MSC_VER
#	define PREFIX ""
#else
#	define PREFIX "lib"
#endif
        static const QString prefix = OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH;
        QString path = prefix + PREFIX "opentrack-proto-simconnect.dll";
        QByteArray name = QFile::encodeName(path);
        actx.lpSource = name.constData();
        hactctx = CreateActCtxA(&actx);
        actctx_cookie = 0;
        if (hactctx != INVALID_HANDLE_VALUE)
        {
            if (!ActivateActCtx(hactctx, &actctx_cookie))
            {
                qDebug() << "simconnect: can't set win32 activation context" << GetLastError();
                ReleaseActCtx(hactctx);
                hactctx = INVALID_HANDLE_VALUE;
            }
            else
                ok = true;
        } else {
            qDebug() << "simconnect: can't create win32 activation context" << GetLastError();
        }
    }
    ~ActivationContext() {
        if (hactctx != INVALID_HANDLE_VALUE)
        {
            DeactivateActCtx(0, actctx_cookie);
            ReleaseActCtx(hactctx);
        }
    }
    bool is_ok() const { return ok; }

private:
    ULONG_PTR actctx_cookie;
    HANDLE hactctx;
    bool ok;
};

module_status simconnect::initialize()
{
    if (!SCClientLib.isLoaded())
    {
        ActivationContext ctx(142 + static_cast<int>(s.sxs_manifest));

        if (ctx.is_ok())
        {
            SCClientLib.setFileName("SimConnect.dll");
            if (!SCClientLib.load())
                return error(tr("dll load failed -- %1").arg(SCClientLib.errorString()));
        }
        else
            return error("can't load SDK -- check selected simconnect version");
    }

    simconnect_open = (importSimConnect_Open) SCClientLib.resolve("SimConnect_Open");
    if (simconnect_open == NULL) {
        return error("simconnect: SimConnect_Open function not found in DLL!");
    }
    simconnect_set6DOF = (importSimConnect_CameraSetRelative6DOF) SCClientLib.resolve("SimConnect_CameraSetRelative6DOF");
    if (simconnect_set6DOF == NULL) {
        return error("simconnect: SimConnect_CameraSetRelative6DOF function not found in DLL!");
    }
    simconnect_close = (importSimConnect_Close) SCClientLib.resolve("SimConnect_Close");
    if (simconnect_close == NULL) {
        return error("simconnect: SimConnect_Close function not found in DLL!");
    }

    simconnect_calldispatch = (importSimConnect_CallDispatch) SCClientLib.resolve("SimConnect_CallDispatch");
    if (simconnect_calldispatch == NULL) {
        return error("simconnect: SimConnect_CallDispatch function not found in DLL!");
    }

    simconnect_subscribetosystemevent = (importSimConnect_SubscribeToSystemEvent) SCClientLib.resolve("SimConnect_SubscribeToSystemEvent");
    if (simconnect_subscribetosystemevent == NULL) {
        return error("simconnect: SimConnect_SubscribeToSystemEvent function not found in DLL!");
    }

    start();

    return status_ok();
}

void simconnect::handle()
{
    (void) simconnect_set6DOF(hSimConnect, virtSCPosX, virtSCPosY, virtSCPosZ, virtSCRotX, virtSCRotZ, virtSCRotY);
}

void CALLBACK simconnect::processNextSimconnectEvent(SIMCONNECT_RECV* pData, DWORD, void *self_)
{
    simconnect& self = *reinterpret_cast<simconnect*>(self_);

    switch(pData->dwID)
    {
    default:
        break;
    case SIMCONNECT_RECV_ID_EXCEPTION:
        qDebug() << "simconnect: got exception";
        //self.should_reconnect = true;
        break;
    case SIMCONNECT_RECV_ID_QUIT:
        qDebug() << "simconnect: got quit event";
        self.should_reconnect = true;
        break;
    case SIMCONNECT_RECV_ID_EVENT_FRAME:
        self.handle();
        break;
    }
}

OPENTRACK_DECLARE_PROTOCOL(simconnect, SCControls, simconnectDll)
