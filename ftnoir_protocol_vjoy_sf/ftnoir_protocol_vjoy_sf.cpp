#include "ftnoir_protocol_vjoy_sf.h"
#include "facetracknoir/plugin-support.h"
#include <ftnoir_tracker_base/ftnoir_tracker_types.h>
#include <QDebug>

/** constructor **/
FTNoIR_Protocol::FTNoIR_Protocol()
{

}

/** destructor **/
FTNoIR_Protocol::~FTNoIR_Protocol()
{
    VjdStat status = GetVJDStatus(s.intvJoyID);
    if(status == VJD_STAT_OWN)
    {
            RelinquishVJD(s.intvJoyID);
            qDebug() << "vJoy SF: Relinquished device.";
    } else {
        qDebug() << "vJoy SF: Can't relinquish device, it's in unappropriate state.";
    }
}

bool FTNoIR_Protocol::checkServerInstallationOK()
{
    // Get the driver attributes (Vendor ID, Product ID, Version Number)
    if (!vJoyEnabled())
    {
        qDebug() << "vJoy SF: Failed getting vJoy attributes. Try to reinstall vJoy drivers";
        return false;
    }

    qDebug() << "vJoy SF: "\
           << "Vendor: " << TEXT(GetvJoyManufacturerString())\
           << "Product: " << TEXT(GetvJoyProductString())\
           << "Version Number: " << TEXT(GetvJoySerialNumberString());

    // Get the state of the requested device
    VjdStat status = GetVJDStatus(s.intvJoyID);
    switch (status)
    {
        case VJD_STAT_OWN:
            qDebug() << "vJoy SF: Device " << TEXT(s.intvJoyID) << " is already owned by this feeder.";
            break;
        case VJD_STAT_FREE:
            qDebug() << "vJoy SF: Device " << TEXT(s.intvJoyID) << " is free.";
            break;
        case VJD_STAT_BUSY:
            qDebug() << "vJoy SF: Device " << TEXT(s.intvJoyID) << " is already owned by another feeder. Cannot continue.";
            return false;
        case VJD_STAT_MISS:
            qDebug() << "vJoy SF: Device " << TEXT(s.intvJoyID) << " is not installed or disabled. Cannot continue.";
            return false;
        default:
            qDebug() << "vJoy SF: Device " << TEXT(s.intvJoyID) << " general error. Cannot continue";
            return false;
    };

    hasAxisX  = GetVJDAxisExist(s.intvJoyID, HID_USAGE_X);
    hasAxisY  = GetVJDAxisExist(s.intvJoyID, HID_USAGE_Y);
    hasAxisZ  = GetVJDAxisExist(s.intvJoyID, HID_USAGE_Z);
    hasAxisRX = GetVJDAxisExist(s.intvJoyID, HID_USAGE_RX);
    hasAxisRX = GetVJDAxisExist(s.intvJoyID, HID_USAGE_RY);
    hasAxisRZ = GetVJDAxisExist(s.intvJoyID, HID_USAGE_RZ);

    if(hasAxisX){
        GetVJDAxisMax(s.intvJoyID, HID_USAGE_X, &lAxesMax[0]);
        GetVJDAxisMin(s.intvJoyID, HID_USAGE_X, &lAxesMin[0]);
    }
    if(hasAxisY){
        GetVJDAxisMax(s.intvJoyID, HID_USAGE_Y, &lAxesMax[1]);
        GetVJDAxisMin(s.intvJoyID, HID_USAGE_Y, &lAxesMin[1]);
    }
    if(hasAxisZ){
        GetVJDAxisMax(s.intvJoyID, HID_USAGE_Z, &lAxesMax[2]);
        GetVJDAxisMin(s.intvJoyID, HID_USAGE_Z, &lAxesMin[2]);
    }
    if(hasAxisRX){
        GetVJDAxisMax(s.intvJoyID, HID_USAGE_RX, &lAxesMax[3]);
        GetVJDAxisMin(s.intvJoyID, HID_USAGE_RX, &lAxesMin[3]);
    }
    if(hasAxisRY){
        GetVJDAxisMax(s.intvJoyID, HID_USAGE_RY, &lAxesMax[4]);
        GetVJDAxisMin(s.intvJoyID, HID_USAGE_RY, &lAxesMin[4]);
    }
    if(hasAxisRZ){
        GetVJDAxisMax(s.intvJoyID, HID_USAGE_RZ, &lAxesMax[5]);
        GetVJDAxisMin(s.intvJoyID, HID_USAGE_RZ, &lAxesMin[5]);
    }

    // Acquire the target
    if ((status == VJD_STAT_OWN) || ((status == VJD_STAT_FREE) && (!AcquireVJD(s.intvJoyID))))
    {
        qDebug() << "vJoy SF: Failed to acquire vJoy device number" << s.intvJoyID;
        return false;
    }
    else
    {
        qDebug() << "vJoy SF: Acquired vJoy device number %d" << s.intvJoyID;
    }

    return true;
}

void FTNoIR_Protocol::sendHeadposeToGame(const double* headpose) {

    // have no idea why it use weird way like that but since it's in the official docs...
    bytevJoyID = (BYTE) s.intvJoyID;
    vJoyPosition.bDevice = bytevJoyID;


    if(hasAxisX)
        vJoyPosition.wAxisX = calcAxisValue(headpose[Yaw], lAxesMax[0], lAxesMin[0], 180);
    if(hasAxisY)
        vJoyPosition.wAxisY = calcAxisValue(headpose[Pitch], lAxesMax[1], lAxesMin[1], 90);
    if(hasAxisZ)
        vJoyPosition.wAxisZ = calcAxisValue(headpose[Roll], lAxesMax[2], lAxesMin[2], 180);
    if(hasAxisRX)
        vJoyPosition.wAxisXRot = calcAxisValue(headpose[TX], lAxesMax[3], lAxesMin[3], 100);
    if(hasAxisRY)
        vJoyPosition.wAxisYRot = calcAxisValue(headpose[TY], lAxesMax[4], lAxesMin[4], 100);
    if(hasAxisRZ)
        vJoyPosition.wAxisZRot = calcAxisValue(headpose[TZ], lAxesMax[5], lAxesMin[5], 100);

    /****
    if(hasAxisX)
        vJoyPosition.wAxisX = std::min<int>(lAxesMax[0], std::max<int>(lAxesMin[0], headpose[Yaw] * lAxesMax[0] / 180.0));
    if(hasAxisY)
        vJoyPosition.wAxisY = std::min<int>(lAxesMax[1], std::max<int>(lAxesMin[1], headpose[Pitch] * lAxesMax[1] / 90.0));
    if(hasAxisZ)
        vJoyPosition.wAxisZ = std::min<int>(lAxesMax[2], std::max<int>(lAxesMin[2], headpose[Roll] * lAxesMax[2] / 180.0));
    if(hasAxisRX)
        vJoyPosition.wAxisXRot = std::min<int>(lAxesMax[3], std::max<int>(lAxesMin[3], headpose[TX] * lAxesMax[3] / 100.0));
    if(hasAxisRY)
        vJoyPosition.wAxisYRot = std::min<int>(lAxesMax[4], std::max<int>(lAxesMin[4], headpose[TY] * lAxesMax[4] / 100.0));
    if(hasAxisRZ)
        vJoyPosition.wAxisZRot = std::min<int>(lAxesMax[5], std::max<int>(lAxesMin[5], headpose[TZ] * lAxesMax[5] / 100.0));
    ***/

    /*** Feed the driver with the position packet
     *   if it fails then wait for input then try to re-acquire device ***/
    if (!UpdateVJD(s.intvJoyID, (PVOID)&vJoyPosition))
    {
        qDebug() << "vJoy SF: feeding device " << s.intvJoyID << " failed. Try to stop Open Track, activate the device and start Open Track again.";
    }

}

/***
 * calc target axis value with given parameters
 */
long FTNoIR_Protocol::calcAxisValue(double rawValue, long axisAbsMax, long axisAbsMin, long halfAngle) {

    long axisValue = 0;
    long axisMiddle = (axisAbsMax - axisAbsMin) / 2;
    long axisAbsMiddle = axisAbsMax - axisMiddle;
    double unit = axisMiddle / halfAngle;

    // if(rawValue >= 0)
    // {
        axisValue = axisAbsMiddle + std::floor(0.5 + unit * rawValue);
    // } else {
    //    axisValue = axisAbsMiddle - std::floor(0.5 + std::abs(unit) * rawValue);
    // }

    return axisValue;
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocol* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Protocol;
}
