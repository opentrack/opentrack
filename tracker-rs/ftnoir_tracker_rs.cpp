/*
 * Copyright (c) 2015, Intel Corporation
 *   Author: Xavier Hallade <xavier.hallade@intel.com>
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "ftnoir_tracker_rs.h"
#include "ftnoir_tracker_rs_controls.h"
#include "imagewidget.h"
#include "opentrack/plugin-api.hpp"
#include <QMessageBox>
#include <QProcess>
#include <QStackedLayout>

RSTracker::RSTracker() {
    connect(&mTrackerWorkerThread, &RSTrackerWorkerThread::trackingHasFinished,
            this, &RSTracker::handleTrackingEnded);

    connect(&mPreviewUpdateTimer, &QTimer::timeout,
            this, &RSTracker::updatePreview);
}

void RSTracker::configurePreviewFrame()
{
    if(mImageWidget!=nullptr || mPreviewFrame==nullptr)
        return;

    mImageWidget = new ImageWidget(mPreviewFrame);
    mImageWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    if(mPreviewFrame->layout() != nullptr){
        delete mPreviewFrame->layout();
    }

    QLayout* layout = new QStackedLayout();
    mPreviewFrame->setLayout(layout);

    layout->addWidget(mImageWidget);
}

void RSTracker::start_tracker(QFrame* previewFrame)
{
    mPreviewFrame = previewFrame;

    mTrackerWorkerThread.start(QThread::HighPriority);

    configurePreviewFrame();

    startPreview();
}

void RSTracker::startPreview(){
    mPreviewUpdateTimer.start(kPreviewUpdateInterval);
}

void RSTracker::updatePreview(){
    if(mImageWidget->isEnabled())
        mImageWidget->setImage(mTrackerWorkerThread.getPreview());
}

void RSTracker::stopPreview(){
    mPreviewUpdateTimer.stop();
}

void RSTracker::handleTrackingEnded(int exitCode){
    stopPreview();

    if(exitCode!=0)
        showRealSenseErrorMessageBox(exitCode);
}

bool RSTracker::startSdkInstallationProcess()
{
    bool pStarted = QProcess::startDetached("contrib\\intel_rs_sdk_runtime_websetup_8.0.24.6528.exe --finstall=core,face3d --fnone=all");
    if(!pStarted){
        QMessageBox::warning(0, "Intel® RealSense™ Runtime Installation", "Installation process failed to start.", QMessageBox::Ok);
    }
    return pStarted;
}

void RSTracker::showRealSenseErrorMessageBox(int exitCode)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText("RealSense Tracking Error");
    if(exitCode==-101){ //The implementation got an invalid handle from the RealSense SDK session/modules
        msgBox.setInformativeText("Couldn't initialize RealSense tracking. Please install SDK Runtime R5.");
    }
    else {
        msgBox.setInformativeText("Status code: " + QString::number(exitCode) + ".\n\nNote that you need the latest camera drivers and the SDK runtime 2016 R1 to be installed.");
    }

    QPushButton* triggerSdkInstallation = msgBox.addButton("Install Runtime", QMessageBox::ActionRole);
    msgBox.addButton(QMessageBox::Ok);
    msgBox.exec();

    if(msgBox.clickedButton() == triggerSdkInstallation)
        startSdkInstallationProcess();
}

void RSTracker::data(double *data)
{
    mTrackerWorkerThread.getPose(data);
}

RSTracker::~RSTracker() {
    stopPreview();

    if (mPreviewFrame!=nullptr && mPreviewFrame->layout()!=nullptr)
        delete mPreviewFrame->layout();

    mTrackerWorkerThread.requestInterruption();
    mTrackerWorkerThread.quit();
    mTrackerWorkerThread.wait();
}

QString RSTrackerMetaData::name() {
    return QString("Intel® RealSense™ Technology");
}

QIcon RSTrackerMetaData::icon() {
    return QIcon(":/images/intel-16x16.png");
}

OPENTRACK_DECLARE_TRACKER(RSTracker, RSTrackerControls, RSTrackerMetaData)
