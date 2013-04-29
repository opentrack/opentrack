#include "ftnoir_protocol_vjoy.h"
#include "facetracknoir/global-settings.h"
#include <ftnoir_tracker_base/ftnoir_tracker_types.h>

/** constructor **/
FTNoIR_Protocol::FTNoIR_Protocol()
{
    VJoy_Initialize("", "");
}

/** destructor **/
FTNoIR_Protocol::~FTNoIR_Protocol()
{
    VJoy_Shutdown();
}

void FTNoIR_Protocol::Initialize()
{
}

void FTNoIR_Protocol::sendHeadposeToGame( double *headpose, double *rawheadpose ) {
    JOYSTICK_STATE state[2] = { 0 };

    state[0].XAxis = std::min<int>(VJOY_AXIS_MAX, std::max<int>(VJOY_AXIS_MIN, headpose[RX] * VJOY_AXIS_MAX / 180.0));
    state[0].YAxis = std::min<int>(VJOY_AXIS_MAX, std::max<int>(VJOY_AXIS_MIN, headpose[RY] * VJOY_AXIS_MAX / 180.0));
    state[0].ZAxis = std::min<int>(VJOY_AXIS_MAX, std::max<int>(VJOY_AXIS_MIN, headpose[RZ] * VJOY_AXIS_MAX / 180.0));
    state[0].XRotation = std::min<int>(VJOY_AXIS_MAX, std::max<int>(VJOY_AXIS_MIN, headpose[TX] * VJOY_AXIS_MAX / 100.0));
    state[0].YRotation = std::min<int>(VJOY_AXIS_MAX, std::max<int>(VJOY_AXIS_MIN, headpose[TY] * VJOY_AXIS_MAX / 100.0));
    state[0].ZRotation = std::min<int>(VJOY_AXIS_MAX, std::max<int>(VJOY_AXIS_MIN, headpose[TZ] * VJOY_AXIS_MAX / 100.0));
    
    VJoy_UpdateJoyState(0, state);
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocol* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Protocol;
}
