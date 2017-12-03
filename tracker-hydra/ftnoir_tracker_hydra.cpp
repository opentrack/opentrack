/* Copyright (c) 2013 mm0zct
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ftnoir_tracker_hydra.h"
#include "api/plugin-api.hpp"
#include <cstdio>
#include <cmath>
#ifdef _WIN32
#   define SIXENSE_STATIC_LIB
#   define SIXENSE_UTILS_STATIC_LIB
#endif
#include <sixense.h>

Hydra_Tracker::Hydra_Tracker() {}

#include <sixense_math.hpp>

Hydra_Tracker::~Hydra_Tracker()
{

    sixenseExit();
}

module_status Hydra_Tracker::start_tracker(QFrame*)
{
    sixenseInit();

    return status_ok();
}

void Hydra_Tracker::data(double *data)
{

    sixenseSetActiveBase(0);
    sixenseAllControllerData acd;
    sixenseGetAllNewestData( &acd );
    sixenseMath::Matrix4 mat = sixenseMath::Matrix4(acd.controllers[0].rot_mat);

    float ypr[3];

    mat.getEulerAngles().fill(ypr);
    data[TX] = double(acd.controllers[0].pos[0])/50;
    data[TY] = double(acd.controllers[0].pos[1])/50;
    data[TZ] = double(acd.controllers[0].pos[2])/50;
    constexpr double r2d = 180/M_PI;
    data[Yaw] = double(ypr[0]) * r2d;
    data[Pitch] = double(ypr[1]) * r2d;
    data[Roll] = double(ypr[2]) * r2d;
}

OPENTRACK_DECLARE_TRACKER(Hydra_Tracker, dialog_hydra, hydraDll)
