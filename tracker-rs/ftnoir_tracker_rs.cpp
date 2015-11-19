/*
 * Copyright (c) 2015, Intel Corporation
 *   Author: Xavier Hallade <xavier.hallade@intel.com>
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "ftnoir_tracker_rs.h"
#include "ftnoir_tracker_rs_controls.h"

#include "opentrack/plugin-api.hpp"
#include <QMessageBox>

RSTracker::RSTracker() : mPose{ 0,0,0, 0,0,0 } {
    mThread.setObjectName("RSTrackerWorkerThread");

    mRealSenseImplProcess.moveToThread(&mThread);
    mSocket.moveToThread(&mThread);

    connect(&mRealSenseImplProcess, SIGNAL(finished(int)),
            this, SLOT(rsImplProcessFinished(int)), Qt::QueuedConnection);

    qRegisterMetaType<QProcess::ProcessError>("QProcess::ProcessError");
    connect(&mRealSenseImplProcess, SIGNAL(error(QProcess::ProcessError)),
            this, SLOT(rsImplProcessError(QProcess::ProcessError)), Qt::QueuedConnection);

    connect(&mSocket, SIGNAL(readyRead()),
            this, SLOT(readPendingUdpPoseData()), Qt::DirectConnection);

    connect(&mThread, &QThread::started,
            &mThread, [this]{
        mSocket.bind(QHostAddress::LocalHost, 4242, QUdpSocket::DontShareAddress);
        mRealSenseImplProcess.start("opentrack-tracker-rs-impl.exe", QProcess::NotOpen);
    }, Qt::DirectConnection);

    connect(&mThread, &QThread::finished,
            &mThread, [this]{
        mRealSenseImplProcess.kill();
        mRealSenseImplProcess.waitForFinished();
    }, Qt::DirectConnection);
}

void RSTracker::start_tracker(QFrame*)
{
    mThread.start();
}

void RSTracker::readPendingUdpPoseData(){
    double pose[6];

    while(mSocket.hasPendingDatagrams()) {
        mSocket.readDatagram((char*)pose, sizeof(pose));
        QMutexLocker foo(&mMutex);
        memcpy(mPose, pose, sizeof(pose));
    }
}

void RSTracker::rsImplProcessError(QProcess::ProcessError error){
    if(error == QProcess::FailedToStart){
        QMessageBox::warning(NULL, "RealSense Tracking Error", "Couldn't start the RealSense tracking module.\nMaybe opentrack-tracker-rs-impl.exe is missing.", QMessageBox::Ok);
    }
    else if(error == QProcess::Crashed){
        QMessageBox::warning(NULL, "RealSense Tracking Error", "The RealSense tracking module has crashed.", QMessageBox::Ok);
    }
}


void RSTracker::rsImplProcessFinished(int exitCode){
    if(exitCode!=0){
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText("RealSense Tracking Error");
        if(exitCode==-101){ //The implementation got an invalid handle from the RealSense SDK session/modules
            msgBox.setInformativeText("Couldn't initialize RealSense tracking. Please install SDK Runtime R5.");
        }
        else {
            msgBox.setInformativeText("Status code: " + QString::number(exitCode) + ".\n\nNote that you need the latest camera drivers and the SDK runtime R5 to be installed.");
        }
        QPushButton* triggerSdkInstallation = msgBox.addButton("Install Runtime", QMessageBox::ActionRole);
        msgBox.addButton(QMessageBox::Ok);
        msgBox.exec();

        if(msgBox.clickedButton() == triggerSdkInstallation){
            bool pStarted = QProcess::startDetached("contrib\\intel_rs_sdk_runtime_websetup_7.0.23.6161.exe --finstall=core,face3d --fnone=all");
            if(!pStarted){
                QMessageBox::warning(0, "Intel® RealSense™ Runtime Installation", "Installation process failed to start.", QMessageBox::Ok);
            }
        }
    }
}

void RSTracker::data(double *data)
{
    QMutexLocker foo(&mMutex);
    memcpy(data, mPose, sizeof(mPose));
}

RSTracker::~RSTracker() {
    mThread.quit();
    mThread.wait();
}

QString RSTrackerMetaData::name() {
    return QString("Intel® RealSense™ Technology");
}

QIcon RSTrackerMetaData::icon() {
    return QIcon(":/images/intel-16x16.png");
}

OPENTRACK_DECLARE_TRACKER(RSTracker, RSTrackerControls, RSTrackerMetaData)
