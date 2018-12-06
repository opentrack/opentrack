/*
 * Copyright (c) 2015, Intel Corporation
 *   Author: Xavier Hallade <xavier.hallade@intel.com>
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "ftnoir_tracker_rs_worker.h"
#include "rs_impl/ftnoir_tracker_rs_impl.h"
#include <cstdlib>
#include <QImage>
#include <QDebug>

RSTrackerWorkerThread::RSTrackerWorkerThread(): mPose{0,0,0,0,0,0}{
    setObjectName("RSTrackerWorkerThread");
    mPreviewRawData = (uchar*)malloc(1*kPreviewStreamWidth*kPreviewStreamHeight); // Y8 format
    memset(mPreviewRawData, 125, kPreviewStreamWidth*kPreviewStreamHeight); //start with a gray image.
}

void RSTrackerWorkerThread::run(){
    double pose[6];
    int retValue;

    retValue = rs_tracker_impl_start();
	qDebug() << "tracker_rs: tracking has started. retValue: " << retValue;

    if(retValue==0)
        emit trackingHasStarted();
    else {
        emit trackingHasFinished(retValue);
        return;
    }

    while(!isInterruptionRequested()){
        retValue = rs_tracker_impl_update_pose(pose);
		if (retValue == 0) { // success
			QMutexLocker lock(&mMutex);
			memcpy(mPose, pose, sizeof(pose));
		}
		else if (retValue != -303) { // pose update failed and not because of a timeout (-303)
			emit trackingHasFinished(retValue);
			break;
		}
		else
			qDebug() << "tracker_rs: timeout in pose update.";
    }

	qDebug() << "tracker_rs: tracking is ending. retValue: " << retValue;

	retValue = rs_tracker_impl_end();

	qDebug() << "tracker_rs: tracking has ended. retValue: " << retValue;
}

void RSTrackerWorkerThread::getPose(double *pose){
    QMutexLocker lock(&mMutex);
    memcpy(pose, mPose, sizeof(mPose));
}

const QImage RSTrackerWorkerThread::getPreview(){
	if(!isInterruptionRequested() && isRunning())
	    rs_tracker_impl_get_preview(mPreviewRawData, kPreviewStreamWidth, kPreviewStreamHeight);
    return QImage((const uchar*)mPreviewRawData, kPreviewStreamWidth, kPreviewStreamHeight, QImage::Format_Grayscale8).copy();//TODO: avoid deep copy?
}

RSTrackerWorkerThread::~RSTrackerWorkerThread() {
    free(mPreviewRawData);
}
