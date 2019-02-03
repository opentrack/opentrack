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
#include "compat/timer.hpp"
#include "compat/library-path.hpp"

simconnect::~simconnect()
{
    requestInterruption();
    wait();
}

void simconnect::run()
{
    HANDLE event = CreateEventA(nullptr, FALSE, FALSE, nullptr);

    if (event == nullptr)
    {
        qDebug() << "simconnect: event create" << GetLastError();
        return;
    }

    while (!isInterruptionRequested())
    {
        HRESULT hr;

        if (SUCCEEDED(hr = simconnect_open(&handle, "opentrack", nullptr, 0, event, 0)))
        {
            if (!SUCCEEDED(hr = simconnect_subscribetosystemevent(handle, 0, "Frame")))
            {
                qDebug() << "simconnect: can't subscribe to frame event:" << hr;
            }

            Timer tm;
            reconnect = false;

            if (SUCCEEDED(hr))
                while (!isInterruptionRequested())
                {
                    if (reconnect)
                        break;

                    if (WaitForSingleObject(event, 100) == WAIT_OBJECT_0)
                    {
                        tm.start();

                        if (!SUCCEEDED(hr = simconnect_calldispatch(handle, processNextSimconnectEvent, reinterpret_cast<void*>(this))))
                        {
                            qDebug() << "simconnect: calldispatch failed:" << hr;
                            break;
                        }
                    }

                    if (reconnect)
                        break;

                    else
                    {
                        const int idle_seconds = (int)tm.elapsed_seconds();
                        constexpr int max_idle_seconds = 2;

                        if (idle_seconds >= max_idle_seconds)
                            break;
                    }
                }

            (void) simconnect_close(handle);
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

class ActivationContext
{
public:
    explicit ActivationContext(int resid);
    ~ActivationContext();

    bool is_ok() const { return ok; }

private:
    ULONG_PTR cookie = 0;
    HANDLE handle = INVALID_HANDLE_VALUE;
    bool ok = false;
};

ActivationContext::ActivationContext(int resid)
{
    ACTCTXA actx = {};
    actx.cbSize = sizeof(actx);
    actx.lpResourceName = MAKEINTRESOURCEA(resid);
    actx.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;
    static const QString prefix = OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH;
    QString path = prefix + OPENTRACK_LIBRARY_PREFIX "opentrack-proto-simconnect." OPENTRACK_LIBRARY_EXTENSION;
    QByteArray name = QFile::encodeName(path);
    actx.lpSource = name.constData();
    handle = CreateActCtxA(&actx);
    if (handle != INVALID_HANDLE_VALUE)
    {
        if (!ActivateActCtx(handle, &cookie))
        {
            qDebug() << "simconnect: can't set win32 activation context" << GetLastError();
            ReleaseActCtx(handle);
            handle = INVALID_HANDLE_VALUE;
        }
        else
            ok = true;
    } else {
        qDebug() << "simconnect: can't create win32 activation context" << GetLastError();
    }
}

ActivationContext::~ActivationContext()
{
    if (handle != INVALID_HANDLE_VALUE)
    {
        DeactivateActCtx(0, cookie);
        ReleaseActCtx(handle);
    }
}

module_status simconnect::initialize()
{
    if (!library.isLoaded())
    {
        ActivationContext ctx(142 + s.sxs_manifest);

        if (ctx.is_ok())
        {
            library.setFileName("SimConnect.dll");
            library.setLoadHints(QLibrary::PreventUnloadHint | QLibrary::ResolveAllSymbolsHint);
            if (!library.load())
                return error(tr("dll load failed: %1").arg(library.errorString()));
        }
        else
            // XXX TODO add instructions for fsx and p3d -sh 20190128
            return error(tr("install FSX SDK"));
    }

    using ptr = void(*)();

    struct {
        const char* name;
        ptr& place;
    } list[] = {
        { "SimConnect_Open", (ptr&)simconnect_open },
        { "SimConnect_CameraSetRelative6DOF", (ptr&)simconnect_set6DOF },
        { "SimConnect_Close", (ptr&)simconnect_close },
        { "SimConnect_CallDispatch", (ptr&)simconnect_calldispatch },
        { "SimConnect_SubscribeToSystemEvent", (ptr&)simconnect_subscribetosystemevent },
    };

    for (auto& x : list)
    {
        x.place = (ptr)library.resolve(x.name);
        if (!x.place)
            return error(tr("can't import %1: %2").arg(x.name, library.errorString()));
    }

    start();

    return {};
}

void simconnect::handler()
{
    (void) simconnect_set6DOF(handle, virtSCPosX, virtSCPosY, virtSCPosZ, virtSCRotX, virtSCRotZ, virtSCRotY);
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
        self.reconnect = true;
        break;
    case SIMCONNECT_RECV_ID_QUIT:
        qDebug() << "simconnect: got quit event";
        self.reconnect = true;
        break;
    case SIMCONNECT_RECV_ID_EVENT_FRAME:
        self.handler();
        break;
    }
}

QString simconnect::game_name()
{
    return tr("FS2004/FSX");
}

OPENTRACK_DECLARE_PROTOCOL(simconnect, simconnect_ui, simconnect_metadata)
