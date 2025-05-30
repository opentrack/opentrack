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
#include "ftnoir_protocol_sc.h"
#include "api/plugin-api.hpp"
#include "compat/timer.hpp"
#include "compat/library-path.hpp"
#include "compat/activation-context.hpp"

#include <cstddef>
#include <cstdint>

using std::intptr_t;

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
        qDebug() << "fsx: create event failed, error code" << GetLastError();
        return;
    }

    constexpr unsigned sleep_time = 5;

    while (!isInterruptionRequested())
    {
        HRESULT hr;
        reconnect = false;
        handle = nullptr;

        if (!SUCCEEDED(hr = simconnect_open(&handle, "opentrack", nullptr, 0, event, 0)))
            qDebug() << "fsx: connect failed, retry in" << sleep_time << "seconds...";
        else
        {
            if (!SUCCEEDED(hr = simconnect_subscribe(handle, 0, "1sec")))
                qDebug() << "fsx: can't subscribe to frame event:" << (void*)(intptr_t)hr;
            else
            {
                while (!isInterruptionRequested())
                {
                    constexpr int max_idle_ms = 2000;

                    if (WaitForSingleObject(event, max_idle_ms) != WAIT_OBJECT_0)
                    {
                        qDebug() << "fsx: timeout reached, reconnecting";
                        break;
                    }

                    if (reconnect.load(std::memory_order_relaxed))
                        break;

                    if (!SUCCEEDED(hr = simconnect_calldispatch(handle, event_handler, (void*)this)))
                    {
                        qDebug() << "fsx: calldispatch failed:" << (void*)(intptr_t)hr;
                        break;
                    }
                 }
            }

            QMutexLocker l(&mtx);
            (void)simconnect_close(handle);
            handle = nullptr;
        }

        for (unsigned k = 0; k < sleep_time * 25; k++)
        {
            if (isInterruptionRequested())
                break;
            Sleep(1000 / 25);
        }
    }

    qDebug() << "simconnect: exit";

    CloseHandle(event);
}

void simconnect::pose(const double* pose, const double*)
{
    data[Pitch] = (float)-pose[Pitch];
    data[Yaw]   = (float)pose[Yaw];
    data[Roll]  = (float)pose[Roll];

    constexpr float to_meters = 1e-2f;
    data[TX] = (float)-pose[TX] * to_meters;
    data[TY] = (float)pose[TY]  * to_meters;
    data[TZ] = (float)-pose[TZ] * to_meters;

    QMutexLocker l(&mtx);
    if (handle)
        (void)simconnect_set6DOF(handle,
            data[TX], data[TY], data[TZ],
            data[Pitch], data[Roll], data[Yaw]);
}

module_status simconnect::initialize()
{
    if (!library.isLoaded())
    {
        constexpr int resource_offset = 142;
        activation_context ctx("opentrack-proto-simconnect" "." OPENTRACK_LIBRARY_EXTENSION,
                               resource_offset + s.sxs_manifest);

        if (ctx)
        {
            library.setFileName("SimConnect.dll");
            library.setLoadHints(QLibrary::PreventUnloadHint | QLibrary::ResolveAllSymbolsHint);
            if (!library.load())
                return error(tr("dll load failed: %1").arg(library.errorString()));
        }
        else
            // XXX TODO add instructions for fsx and p3d -sh 20190128
            return error(tr("Install FSX/Prepar3D SimConnect SDK."));
    }

    using ptr = decltype(library.resolve(""));

    struct {
        const char* name;
        ptr& place;
    } list[] = {
        { "SimConnect_Open",                   (ptr&)simconnect_open },
        { "SimConnect_CameraSetRelative6DOF",  (ptr&)simconnect_set6DOF },
        { "SimConnect_Close",                  (ptr&)simconnect_close },
        { "SimConnect_CallDispatch",           (ptr&)simconnect_calldispatch },
        { "SimConnect_SubscribeToSystemEvent", (ptr&)simconnect_subscribe },
    };

    for (auto& x : list)
    {
        x.place = library.resolve(x.name);
        if (!x.place)
            return error(tr("can't import %1: %2").arg(x.name, library.errorString()));
    }

    start();

    return {};
}

void simconnect::event_handler(SIMCONNECT_RECV* pData, DWORD, void* self_)
{
    simconnect& self = *reinterpret_cast<simconnect*>(self_);

    switch(pData->dwID)
    {
    default:
        break;
    case SIMCONNECT_RECV_ID_EXCEPTION:
        // CAVEAT: can't reconnect here, it breaks Prepar3D.
        // the timer on the event handle will take care of failures.
        break;
    case SIMCONNECT_RECV_ID_QUIT:
        qDebug() << "fsx: got quit event";
        self.reconnect = true;
        break;
    }
}

QString simconnect::game_name()
{
    return tr("FSX / Prepar3D");
}

OPENTRACK_DECLARE_PROTOCOL(simconnect, simconnect_ui, simconnect_metadata)
