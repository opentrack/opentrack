#include "ftnoir_protocol_vjoy_sf.h"
#include "facetracknoir/plugin-support.h"
#include <ftnoir_tracker_base/ftnoir_tracker_types.h>
#include <QDebug>

/** constructor **/
FTNoIR_Protocol::FTNoIR_Protocol()
{
    intvJoyID = s.intvJoyID;
}

/** destructor **/
FTNoIR_Protocol::~FTNoIR_Protocol()
{
    VjdStat status = GetVJDStatus(intvJoyID);
    if(status == VJD_STAT_OWN)
    {
            RelinquishVJD(intvJoyID);
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
    VjdStat status = GetVJDStatus(intvJoyID);
    switch (status)
    {
        case VJD_STAT_OWN:
            qDebug() << "vJoy SF: Device " << TEXT(intvJoyID) << " is already owned by this feeder.";
            break;
        case VJD_STAT_FREE:
            qDebug() << "vJoy SF: Device " << TEXT(intvJoyID) << " is free.";
            break;
        case VJD_STAT_BUSY:
            qDebug() << "vJoy SF: Device " << TEXT(intvJoyID) << " is already owned by another feeder. Cannot continue.";
            return false;
        case VJD_STAT_MISS:
            qDebug() << "vJoy SF: Device " << TEXT(intvJoyID) << " is not installed or disabled. Cannot continue.";
            return false;
        default:
            qDebug() << "vJoy SF: Device " << TEXT(intvJoyID) << " general error. Cannot continue";
            return false;
    };

    hasAxisX  = GetVJDAxisExist(intvJoyID, HID_USAGE_X);
    hasAxisY  = GetVJDAxisExist(intvJoyID, HID_USAGE_Y);
    hasAxisZ  = GetVJDAxisExist(intvJoyID, HID_USAGE_Z);
    hasAxisRX = GetVJDAxisExist(intvJoyID, HID_USAGE_RX);
    hasAxisRX = GetVJDAxisExist(intvJoyID, HID_USAGE_RY);
    hasAxisRZ = GetVJDAxisExist(intvJoyID, HID_USAGE_RZ);

    if(hasAxisX){
        GetVJDAxisMax(intvJoyID, HID_USAGE_X, &lAxesMax[0]);
        GetVJDAxisMin(intvJoyID, HID_USAGE_X, &lAxesMin[0]);
    }
    if(hasAxisY){
        GetVJDAxisMax(intvJoyID, HID_USAGE_Y, &lAxesMax[1]);
        GetVJDAxisMin(intvJoyID, HID_USAGE_Y, &lAxesMin[1]);
    }
    if(hasAxisZ){
        GetVJDAxisMax(intvJoyID, HID_USAGE_Z, &lAxesMax[2]);
        GetVJDAxisMin(intvJoyID, HID_USAGE_Z, &lAxesMin[2]);
    }
    if(hasAxisRX){
        GetVJDAxisMax(intvJoyID, HID_USAGE_RX, &lAxesMax[3]);
        GetVJDAxisMin(intvJoyID, HID_USAGE_RX, &lAxesMin[3]);
    }
    if(hasAxisRY){
        GetVJDAxisMax(intvJoyID, HID_USAGE_RY, &lAxesMax[4]);
        GetVJDAxisMin(intvJoyID, HID_USAGE_RY, &lAxesMin[4]);
    }
    if(hasAxisRZ){
        GetVJDAxisMax(intvJoyID, HID_USAGE_RZ, &lAxesMax[5]);
        GetVJDAxisMin(intvJoyID, HID_USAGE_RZ, &lAxesMin[5]);
    }

    // Acquire the target
    if ((status == VJD_STAT_OWN) || ((status == VJD_STAT_FREE) && (!AcquireVJD(intvJoyID))))
    {
        qDebug() << "vJoy SF: Failed to acquire vJoy device number" << intvJoyID;
        return false;
    }
    else
    {
        qDebug() << "vJoy SF: Acquired vJoy device number %d" << intvJoyID;
    }

    return true;
}

void FTNoIR_Protocol::sendHeadposeToGame(const double* headpose) {

    // have no idea why it use weird way like that but since it's in the official docs...
    bytevJoyID = (BYTE) intvJoyID;
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

    /*** Feed the driver with the position packet
     *   if it fails then wait for input then try to re-acquire device ***/
    if (!UpdateVJD(intvJoyID, (PVOID)&vJoyPosition))
    {
        qDebug() << "vJoy SF: feeding device " << intvJoyID << " failed. Try to stop Open Track, activate the device and start Open Track again.";
    }

}

/***
 * calc target axis value with given parameters
 */
LONG FTNoIR_Protocol::calcAxisValue(DOUBLE rawValue, LONG axisAbsMax, LONG axisAbsMin, LONG halfAngle) {

    LONG axisValue = 0;
    LONG axisMiddle = (axisAbsMax - axisAbsMin) / 2;
    LONG axisAbsMiddle = axisAbsMax - axisMiddle;
    DOUBLE unit = axisMiddle / halfAngle;

    axisValue = axisAbsMiddle + std::floor(0.5 + unit * rawValue);

    return axisValue;
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocol* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Protocol;
}
