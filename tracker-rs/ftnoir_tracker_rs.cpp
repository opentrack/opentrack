/*
 * Copyright (c) 2015-2016, Intel Corporation
 *   Author: Xavier Hallade <xavier.hallade@intel.com>
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "ftnoir_tracker_rs.h"
#include "ftnoir_tracker_rs_controls.h"
#include "imagewidget.h"
#include "api/plugin-api.hpp"
#include "opentrack-library-path.h"
#include <QMessageBox>
#include <QProcess>
#include <QStackedLayout>
#include <QDebug>

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

    mPreviewFrame->show();

    mImageWidget = new ImageWidget(mPreviewFrame);
    mImageWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    if(mPreviewFrame->layout() != nullptr){
        delete mPreviewFrame->layout();
    }

    QLayout* layout = new QStackedLayout();
    mPreviewFrame->setLayout(layout);
    layout->addWidget(mImageWidget);

    mImageWidget->show();
}

module_status RSTracker::start_tracker(QFrame* previewFrame)
{
        qDebug() << "tracker_rs: starting tracker";

    mPreviewFrame = previewFrame;

    configurePreviewFrame();

    startPreview();

    mTrackerWorkerThread.start(QThread::HighPriority);

    return status_ok();
}

void RSTracker::startPreview(){
        qDebug() << "tracker_rs: starting preview";
    mPreviewUpdateTimer.start(kPreviewUpdateInterval);
}

void RSTracker::updatePreview(){
        if (mImageWidget != nullptr && mImageWidget->isEnabled() && mTrackerWorkerThread.isRunning())
                mImageWidget->setImage(mTrackerWorkerThread.getPreview());
        else
                qDebug() << "tracker_rs: not updating preview. worker thread running: " << mTrackerWorkerThread.isRunning();
}

void RSTracker::stopPreview(){
        mPreviewUpdateTimer.stop();
        qDebug() << "tracker_rs: stopped preview";
}

void RSTracker::handleTrackingEnded(int exitCode){
    stopPreview();

    if(exitCode!=0)
        showRealSenseErrorMessageBox(exitCode);
}

bool RSTracker::startSdkInstallationProcess()
{
    static const QString contrib_path(OPENTRACK_BASE_PATH + OPENTRACK_CONTRIB_PATH);

	bool pStarted = QProcess::startDetached(contrib_path + "intel_rs_sdk_runtime_websetup_10.0.26.0396.exe", QStringList({ "--finstall=core,face3d","--fnone=all" }));
    if(!pStarted){
        QMessageBox::warning(nullptr,
                             tr("Intel® RealSense™ Runtime Installation"),
                             tr("Installation process failed to start."),
                             QMessageBox::Ok);
    }
    return pStarted;
}

void RSTracker::showRealSenseErrorMessageBox(int exitCode)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText("RealSense Tracking Error");

        switch(exitCode){
        case -101: //The implementation got an invalid handle from the RealSense SDK session/modules
        msgBox.setInformativeText(tr("Couldn't initialize RealSense tracking. Please make sure SDK Runtime 2016 R2 is installed."));
                break;
        case -301: //RealSense SDK runtime execution aborted.
                msgBox.setInformativeText(tr("Tracking stopped after the RealSense SDK Runtime execution has aborted."));
                break;
        case -601: //RealSense Camera stream configuration has changed.
                msgBox.setInformativeText(tr("Tracking stopped after another program changed camera streams configuration."));
                break;
        default:
        msgBox.setInformativeText("Status code: " + QString::number(exitCode) + ".\n\nNote that you need the latest camera drivers and the SDK runtime 2016 R2 to be installed.");
    }

    QPushButton* triggerSdkInstallation = msgBox.addButton(tr("Install Runtime"), QMessageBox::ActionRole);
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
        qDebug() << "tracker is being destroyed.";

    stopPreview();

    if(mImageWidget!=nullptr)
        delete mImageWidget;

    if (mPreviewFrame!=nullptr && mPreviewFrame->layout()!=nullptr)
        delete mPreviewFrame->layout();

    mTrackerWorkerThread.requestInterruption();
    mTrackerWorkerThread.quit();
    mTrackerWorkerThread.wait();
}

QString RSTrackerMetaData::name() {
    return otr_tr("Intel® RealSense™ Technology");
}

QIcon RSTrackerMetaData::icon() {
    return QIcon(":/images/intel-16x16.png");
}

OPENTRACK_DECLARE_TRACKER(RSTracker, RSdialog_realsense, RSTrackerMetaData)
