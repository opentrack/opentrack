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

fsuipc::fsuipc() = default;

fsuipc::~fsuipc()
{
    FSUIPC_Close();
}

template<typename t>
int fsuipc::scale(t x, t min_x, t max_x)
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

#if 0
template<typename t>
static bool check_float_fresh(t x, t y)
{
    constexpr t eps = t(1e-4);
    return std::fabs(x - y) >= eps;
}
#endif

void fsuipc::pose(const double *headpose, const double*)
{
    DWORD result;
    WORD FSZoom;

    // cm, X and Y are not working for FS2002/2004!
    double pos_z = headpose[TZ];
    state pitch, yaw, roll; // NOLINT(cppcoreguidelines-pro-type-member-init)

    // offsets derived from freetrack
    pitch.Control = 66503;
    pitch.Value = scale(-headpose[Pitch], -180., 180.); // degrees

    yaw.Control = 66504;
    yaw.Value = scale(headpose[Yaw], -180., 180.);

    roll.Control = 66505;
    roll.Value = scale(headpose[Roll], -180., 180.);

#if 0
    // Only do this when the data has changed. This way, the HAT-switch can be used when tracking is OFF.
    if (check_float_fresh(prevRotX, virtRotX) ||
        check_float_fresh(prevRotY, virtRotY) ||
        check_float_fresh(prevRotZ, virtRotZ) ||
        check_float_fresh(prevPosX, virtPosX) ||
        check_float_fresh(prevPosY, virtPosY) ||
        check_float_fresh(prevPosZ, virtPosZ))
#endif
    {
        FSUIPC_Open(SIM_ANY, &result);

        //
        // Check the FS-version
        //
        if  (((result == FSUIPC_ERR_OK) || (result == FSUIPC_ERR_OPEN)) &&
             ((FSUIPC_FS_Version == SIM_FS2K2) || (FSUIPC_FS_Version == SIM_FS2K4)))
        {
            // Write the 4! DOF-data to FS. Only rotations and zoom are possible.

            FSUIPC_Write(0x3110, 8, &pitch, &result);
            FSUIPC_Write(0x3110, 8, &yaw, &result);
            FSUIPC_Write(0x3110, 8, &roll, &result);

            FSZoom = WORD(pos_z + 64);
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

#if 0
    prevPosX = virtPosX;
    prevPosY = virtPosY;
    prevPosZ = virtPosZ;
    prevRotX = virtRotX;
    prevRotY = virtRotY;
    prevRotZ = virtRotZ;
#endif
}

module_status fsuipc::initialize()
{
#if 0
    FSUIPCLib.setFileName( s.LocationOfDLL );
    FSUIPCLib.setLoadHints(QLibrary::PreventUnloadHint);

    if (!FSUIPCLib.load())
        return error(tr("Can't load fsuipc at '%1'").arg(s.LocationOfDLL));
    else
        return status_ok();
#else
    return {};
#endif
}

OPENTRACK_DECLARE_PROTOCOL(fsuipc, FSUIPCControls, fsuipcDll)
