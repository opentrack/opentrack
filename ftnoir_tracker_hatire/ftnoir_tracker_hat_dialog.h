#ifndef FTNOIR_TRACKER_HAT_DIALOG_H
#define FTNOIR_TRACKER_HAT_DIALOG_H

#ifdef OPENTRACK_API
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#else
#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#endif
#include "ftnoir_tracker_hat_settings.h"
#include "ftnoir_tracker_hat.h"
#include "ui_ftnoir_hatcontrols.h"
#include <QObject>
#include <QTime>
#include <QTimer>
#include <QMessageBox>
#include <QMetaType>

// Widget that has controls for FTNoIR protocol client-settings.
class TrackerControls: public QWidget, public ITrackerDialog
{
    Q_OBJECT
public:
	explicit TrackerControls();
    void registerTracker(ITracker *tracker) virt_override;
    void unRegisterTracker() virt_override;
private:
	Ui::UIHATControls ui;
	FTNoIR_Tracker *theTracker;
	QTime last_time;
	
public slots:
    void WriteMsgInfo(const QByteArray &MsgInfo);

protected slots:
    void doOK();
	void doCancel();
	void doSave();
	void doReset();
	void doCenter();
	void doZero();
	void doSend();
	void poll_tracker_info();
	void doSerialInfo();

protected:
	TrackerSettings settings;
	QTimer timer;
};

#endif //FTNOIR_TRACKER_HAT_DIALOG_H
