/* Homepage         http://facetracknoir.sourceforge.net/home/default.htm        *
 *                                                                               *
 * ISC License (ISC)                                                             *
 *                                                                               *
 * Copyright (c) 2015, Wim Vriend                                                *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */
#include "ftnoir_protocol_fsuipc.h"
#include "api/plugin-api.hpp"

#include <cmath>

fsuipc::fsuipc()
{
    prevPosX = 0.0f;
    prevPosY = 0.0f;
    prevPosZ = 0.0f;
    prevRotX = 0.0f;
    prevRotY = 0.0f;
    prevRotZ = 0.0f;
}

fsuipc::~fsuipc()
{
    FSUIPC_Close();
    FSUIPCLib.unload();
}

template<typename t>
int fsuipc::scale2AnalogLimits(t x, t min_x, t max_x)
{
    t local_x = x;

    if (local_x > max_x)
    {
        local_x = max_x;
    }
    if (local_x < min_x)
    {
        local_x = min_x;
    }

    t y = (16383 * local_x) / max_x;

    return (int) y;
}

template<typename t>
static inline bool check_float_fresh(t x, t y)
{
    constexpr t eps = t(1e-4);
    return std::fabs(x - y) >= eps;
}

void fsuipc::pose(const double *headpose ) {
    DWORD result;
    TFSState pitch;
    TFSState yaw;
    TFSState roll;
    WORD FSZoom;

    double virtPosX;
    double virtPosY;
    double virtPosZ;

    double virtRotX;
    double virtRotY;
    double virtRotZ;

    //	qDebug() << "FSUIPCServer::run() says: started!";

    virtRotX = -headpose[Pitch];				// degrees
    virtRotY = headpose[Yaw];
    virtRotZ = headpose[Roll];

    virtPosX = 0.0f;											// cm, X and Y are not working for FS2002/2004!
    virtPosY = 0.0f;
    virtPosZ = headpose[TZ];

    //
    // Init. the FSUIPC offsets (derived from Free-track...)
    //
    pitch.Control = 66503;
    yaw.Control = 66504;
    roll.Control = 66505;

    //
    // Only do this when the data has changed. This way, the HAT-switch can be used when tracking is OFF.
    //
    if (check_float_fresh(prevRotX, virtRotX) ||
        check_float_fresh(prevRotY, virtRotY) ||
        check_float_fresh(prevRotZ, virtRotZ) ||
        check_float_fresh(prevPosX, virtPosX) ||
        check_float_fresh(prevPosY, virtPosY) ||
        check_float_fresh(prevPosZ, virtPosZ))
    {
        //
        // Open the connection
        //
        FSUIPC_Open(SIM_ANY, &result);

        //
        // Check the FS-version
        //
        if  (((result == FSUIPC_ERR_OK) || (result == FSUIPC_ERR_OPEN)) &&
             ((FSUIPC_FS_Version == SIM_FS2K2) || (FSUIPC_FS_Version == SIM_FS2K4)))
        {
            // Write the 4! DOF-data to FS. Only rotations and zoom are possible.

            pitch.Value = scale2AnalogLimits(virtRotX, -180., 180.);
            FSUIPC_Write(0x3110, 8, &pitch, &result);

            yaw.Value = scale2AnalogLimits(virtRotY, -180., 180.);
            FSUIPC_Write(0x3110, 8, &yaw, &result);

            roll.Value = scale2AnalogLimits(virtRotZ, -180., 180.);
            FSUIPC_Write(0x3110, 8, &roll, &result);

            FSZoom = WORD(virtPosZ + 64);
            FSUIPC_Write(0x832E, 2, &FSZoom, &result);

            //
            // Write the data, in one go!
            //
            FSUIPC_Process(&result);
            if (result == FSUIPC_ERR_SENDMSG)
            {
                // FSUIPC checks for already open connections and returns FSUIPC_ERR_OPEN in that case
                // the connection scope is global for the process. this is why above code doesn't
                // leak resources or have logic errors. see: http://www.purebasic.fr/english/viewtopic.php?t=31112
                FSUIPC_Close();
            }
        }
    }

    prevPosX = virtPosX;
    prevPosY = virtPosY;
    prevPosZ = virtPosZ;
    prevRotX = virtRotX;
    prevRotY = virtRotY;
    prevRotZ = virtRotZ;
}

module_status fsuipc::initialize()
{
    FSUIPCLib.setFileName( s.LocationOfDLL );

    if (FSUIPCLib.load() != true)
        return error(otr_tr("Can't load fsuipc at '%1'").arg(s.LocationOfDLL));
    else
        return status_ok();
}

OPENTRACK_DECLARE_PROTOCOL(fsuipc, FSUIPCControls, fsuipcDll)
