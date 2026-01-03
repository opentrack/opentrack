/* Copyright (c) 2014, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "wxr.h"
#include "api/plugin-api.hpp"
#include "compat/math-imports.hpp"

#include <QDebug>
#include <cmath>

#include <Winsock2.h>
#include <condition_variable>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <iostream>
#include <locale>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <ws2tcpip.h>

wxr_tracker::wxr_tracker()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    HMDRawQuat = QVector4D(0, 0, 0, 0);
    HMDPos = QVector3D(0, 0, 0);

    std::filesystem::path tmpPath = "Z:/";
    std::filesystem::path dirPath = "Z:/tmp/xr";
    std::filesystem::path fallbackDir = "D:/xrtemp";
    std::filesystem::path filePath = dirPath / "vr";
    std::filesystem::path fallbackFile = fallbackDir / "vr";

    std::filesystem::path versionPath = dirPath / "version";
    std::filesystem::path fallbackVersion = fallbackDir / "version";

    if (std::filesystem::exists(tmpPath) && std::filesystem::is_directory(tmpPath))
    {
        if (std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath))
        {
            // Nothing to do
        }
        else
        {
            // Try to create the folder
            try
            {
                std::filesystem::create_directories(dirPath);
            }
            catch (const std::exception& e)
            {
                qWarning() << "[WinXrApi] Error creating tmp/xr directory: " << e.what();
            }
        }

        try
        {
            std::ofstream verFile(versionPath);
            if (verFile.is_open())
            {
                verFile << "0.3";
                verFile.close();
            }
            else
            {
            }
        }
        catch (const std::exception& e)
        {
            qWarning() << "[WinXrApi] Error writing VERSION file: " << e.what();
        }

        try
        {
            std::ofstream file(filePath);
            if (file.is_open())
            {
                file << "VR";
                file.close();
            }
            else
            {
            }
        }
        catch (const std::exception& e)
        {
            qWarning() << "[WinXrApi] Error writing VR file: " << e.what();
        }
    }
    else
    {
        try
        {
            std::ofstream verFile(fallbackVersion);
            if (verFile.is_open())
            {
                verFile << "0.2";
                verFile.close();
            }
            else
            {
            }
        }
        catch (const std::exception& e)
        {
            qWarning() << "[WinXrApi] Error writing test VERSION file: " << e.what();
        }

        try
        {
            std::ofstream file(fallbackFile);
            if (file.is_open())
            {
                file << "VR";
                file.close();
            }
            else
            {
            }
        }
        catch (const std::exception& e)
        {
            qWarning() << "[WinXrApi] Error writing test VR file: " << e.what();
        }

        dirPath = fallbackDir;
    }

    /*std::filesystem::path sysinfoPath = dirPath / "system";

    if (std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath))
    {
        if (std::filesystem::exists(sysinfoPath) && std::filesystem::is_regular_file(sysinfoPath))
        {
            try
            {
                std::ifstream sysInfoFile(sysinfoPath);

                std::string hmdMakeStr;
                std::string hmdModelStr;

                if (sysInfoFile.is_open())
                {
                    std::getline(sysInfoFile, hmdMakeStr);
                    std::getline(sysInfoFile, hmdModelStr);
                    sysInfoFile.close();
                }

                hmdMake = hmdMakeStr;
                hmdModel = hmdModelStr;
            }
            catch (const std::filesystem::filesystem_error& e)
            {
            }
            catch (const std::exception& e)
            {
            }
        }
    }*/

    /*if (hmdMake.empty())
    {
        hmdMake = "META";
    }
    else if (hmdMake == "OCULUS")
    {
        hmdMake = "META";
    }*/

    // if (hmdModel.empty())
    //{
    //     hmdModel = "QUEST 3";
    // }
    // else if (hmdModel == "EUREKA" || hmdModel == "PANTHER")
    //{
    //     hmdModel = "QUEST 3";
    // }
    // else if (hmdMake == "META")
    //{
    //     // SEACLIFF - Quest Pro - Untested, assuming hands upside down
    //     // HOLLYWOOD - Quest 2 - Works! Hands upside down, performance is OK (much better with Turnip driver)
    //     // MONTEREY - Quest 1 - Untested, unsupported
    //     hmdModel = "QUEST 2";
    // }

    udpReadThread = std::thread(&wxr_tracker::ReceiveData, this);
    udpReadThread.detach();

    std::string aerMode = "-1";

    // AER is not likely necessary for non-VR apps, they can use SBS via reshade most often
    // if (bEnableAltEyeRendering)
    //{
    //     aerMode = "2";
    // }

    float targetFOVH = 104.5;
    float targetFOVW = 104.5;

    SendData("0 0 2 " + aerMode + " " + std::to_string(targetFOVH) + " " + std::to_string(targetFOVW));
}
wxr_tracker::~wxr_tracker()
{
    KillReceiver();
}

void wxr_tracker::ReceiveData()
{
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serverAddr, clientAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(udpPort);

    try
    {
        bind(udpSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    }
    catch (const std::exception& e)
    {
        qWarning() << "[WinXrUDP] Error starting UDP receiver: " << e.what();
    }

    while (true)
    {
        try
        {
            char buffer[1024];
            int addrLen = sizeof(clientAddr);
            ptrdiff_t bytesReceived = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &addrLen);

            if (bytesReceived > 0 && bytesReceived < 1024)
            {
                buffer[bytesReceived] = '\0';
                std::string returnData(buffer);

                std::istringstream iss(returnData);
                std::string client;
                std::vector<float> floats(28);
                int openXRFrameID;

                std::locale c_locale("C");
                iss.imbue(c_locale);

                iss >> client;
                for (auto& f : floats)
                {
                    iss >> f;
                }
                iss >> openXRFrameID;

                // if (OpenXRFrameID == openXRFrameID)
                //{
                // OpenXRFrameWait = 1;
                //    continue;
                //}
                // else
                //{
                // OpenXRFrameWait = 0;
                //}

                // qWarning() << "[WinXrUDP] UDP DATA " + returnData;

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    retData = returnData;
                }
                cv.notify_all();
            }
        }
        catch (const std::exception& e)
        {
            qWarning() << "[WinXrUDP] Error receiving UDP data: " << e.what();
        }
    }
}

void wxr_tracker::SendData(std::string sendData)
{
    try
    {
        /*WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            qWarning() << "[WinXrUDP] WSAStartup failed with error " << WSAGetLastError();
            return;
        }*/

        struct sockaddr_in targetAddress;
        udpSendSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (udpSendSocket == INVALID_SOCKET)
        {
            qWarning() << "[WinXrUDP] Error sending UDP data: socket creation failed";
            return;
        }

        targetAddress.sin_family = AF_INET;
        targetAddress.sin_port = htons(udpSendPort);
        inet_pton(AF_INET, "127.0.0.1", &targetAddress.sin_addr);

        int result = sendto(udpSendSocket, sendData.c_str(), sendData.length(), 0, (struct sockaddr*)&targetAddress, sizeof(targetAddress));
        if (result == SOCKET_ERROR)
        {
            qWarning() << "[WinXrUDP] sendto failed with error " << WSAGetLastError();
        }

        closesocket(udpSendSocket);
    }
    catch (const std::exception& e)
    {
        qWarning() << "[WinXrUDP] Error sending UDP data: " << e.what();
    }
}

void wxr_tracker::KillReceiver()
{
    // qWarning() << "[WinXrUDP] Shutting down UDP receiver...";

    try
    {
        udpReadThread.~thread();
        udpReadThread = std::thread();
        closesocket(udpSocket);
        WSACleanup();
    }
    catch (const std::exception& e)
    {
        qWarning() << "[WinXrUDP] Error killing UDP receiver: " << e.what();
    }
}

std::string wxr_tracker::GetRetData()
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this] { return !retData.empty(); });
    return retData;
}

module_status wxr_tracker::start_tracker(QFrame*)
{
    t.start();
    return {};
}

QVector4D wxr_tracker::QuaternionMultiply(const QVector4D& q1, const QVector4D& q2)
{
    return QVector4D(q1.w() * q2.x() + q1.x() * q2.w() + q1.y() * q2.z() - q1.z() * q2.y(),
                     q1.w() * q2.y() - q1.x() * q2.z() + q1.y() * q2.w() + q1.z() * q2.x(),
                     q1.w() * q2.z() + q1.x() * q2.y() - q1.y() * q2.x() + q1.z() * q2.w(),
                     q1.w() * q2.w() - q1.x() * q2.x() - q1.y() * q2.y() - q1.z() * q2.z());
}

void wxr_tracker::data(double* data)
{
    // const double dt = t.elapsed_seconds();
    // t.start();

    // for (int i = 0; i < 3; i++)
    //{
    //     double last_ = last[i];
    //     double max = max_values[i] * 2;
    //     double incr_ = incr[i];
    //     double x = fmod(last_ + incr_ * dt, max);
    //     last[i] = x;
    //     if (x > max_values[i])
    //         x = -max + x;
    //     data[i+3] = x;
    // }

    // data[0] = t.elapsed_seconds();

    // qWarning() << "[WinXrApi] Getting UDP Data...";

    std::string txt = GetRetData();

    //qWarning() << txt;

    std::istringstream iss(txt);
    std::string client;
    std::vector<float> floats(28);
    int openXRFrameID;
    std::string buttonString;

    std::locale c_locale("C");
    iss.imbue(c_locale);

    // Parse client string
    iss >> client;

    // Parse float values
    for (auto& f : floats)
    {
        iss >> f;
    }

    // Parse integer value
    iss >> openXRFrameID >> buttonString;

    std::vector<bool> buttonBools;

    if (buttonString.empty())
    {
        buttonString = "FFFFFFFFFFFFFFFFFFFFF";
    }

    for (char c : buttonString)
    {
        if (c == 'F')
        {
            buttonBools.push_back(false);
        }
        else if (c == 'T')
        {
            buttonBools.push_back(true);
        }
    }

    isImmersive = buttonBools[19];
    isSBS = buttonBools[20];

    HMDRawQuat = QVector4D(floats[18], floats[19], -floats[20], floats[21]);
    HMDPos = QVector3D(floats[22], floats[23], floats[24]);

    //QVector4D rollInversion = QVector4D(0.0f, 0.0f, 1.0f, 0.0f); // Quaternion for 180-degree rotation around Z-axis
    //QVector4D quat = QuaternionMultiply(HMDRawQuat, rollInversion);

    // data[1] = openXRFrameID;
    data[0] = floats[22] * -20.0;
    data[1] = floats[23] * 20.0;
    data[2] = floats[24] * 20.0;

    QQuaternion hmdQuat = QQuaternion::QQuaternion(HMDRawQuat); // quat);
    QVector3D hmdEuler = hmdQuat.toEulerAngles();

    if (isImmersive)
    {
        data[3] = 360.0 - (hmdEuler.y() * s.yaw_scale_immersive);
        data[4] = hmdEuler.x() * s.pitch_scale_immersive;
        data[5] = hmdEuler.z() * s.roll_scale_immersive;
    }
    else
    {
        data[3] = 360.0 - (hmdEuler.y() * s.yaw_scale);
        data[4] = hmdEuler.x() * s.pitch_scale;
        data[5] = hmdEuler.z() * s.roll_scale;
    }
}

OPENTRACK_DECLARE_TRACKER(wxr_tracker, wxr_dialog, wxr_metadata)
