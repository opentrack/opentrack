#ifndef FTNOIR_TRACKER_HAT_DIALOG_H
#define FTNOIR_TRACKER_HAT_DIALOG_H

#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#include "ftnoir_tracker_hat_settings.h"
#include "ftnoir_tracker_hat.h"
#include "ui_ftnoir_hatcontrols.h"

#include <QTimer>
#include <QMessageBox>

// Widget that has controls for FTNoIR protocol client-settings.
class TrackerControls: public QWidget, Ui::UIHATControls, public ITrackerDialog
{
    Q_OBJECT

public:
	explicit TrackerControls();
    virtual ~TrackerControls();

    void Initialize(QWidget *parent);
	void registerTracker(ITracker *tracker);
	void unRegisterTracker() ;

private:
	Ui::UIHATControls ui;
	int pre_trame;
	FTNoIR_Tracker *theTracker;

protected slots:

	/*
	void set_mod_port(int val)	 {  settings.SerialPortName = ui.cbSerialPort->itemText(val);    
	                                QMessageBox::warning(this,"FaceTrackNoIR Error", settings.SerialPortName);
	                                settings_changed(); }
*/
	void set_mod_port(const QString & val)	 {  settings.SerialPortName =val;    
	                                QMessageBox::warning(this,"FaceTrackNoIR Error", settings.SerialPortName);
	                                settings_changed(); }
	void set_ena_roll(bool val)	 { settings.EnableRoll = val;    settings_changed(); }
	void set_ena_pitch(bool val) { settings.EnablePitch = val;   settings_changed(); }
	void set_ena_yaw(bool val)	 { settings.EnableYaw = val;     settings_changed(); }
	void set_ena_x(bool val)	 { settings.EnableX = val;       settings_changed(); }
	void set_ena_y(bool val)	 { settings.EnableY = val;       settings_changed(); }
	void set_ena_z(bool val)	 { settings.EnableZ = val;       settings_changed(); }

	void set_inv_roll(bool val)	 { settings.InvertRoll = val;    settings_changed(); }
	void set_inv_pitch(bool val) { settings.InvertPitch = val;   settings_changed(); }
	void set_inv_yaw(bool val)	 { settings.InvertYaw = val;     settings_changed(); }
	void set_inv_x(bool val)	 { settings.InvertX = val;       settings_changed(); }
	void set_inv_y(bool val)	 { settings.InvertY = val;       settings_changed(); }
	void set_inv_z(bool val)	 { settings.InvertZ = val;       settings_changed(); }


	void set_rot_roll(int val)	 { settings.RollAxe = val;    settings_changed(); }
	void set_rot_pitch(int val)	 { settings.PitchAxe = val;   settings_changed(); }
	void set_rot_yaw(int val)	 { settings.YawAxe = val;     settings_changed(); }
	void set_acc_x(int val)		 { settings.XAxe = val;       settings_changed(); }
	void set_acc_y(int val)		 { settings.YAxe = val;       settings_changed(); }
	void set_acc_z(int val)		 { settings.ZAxe = val;       settings_changed(); }

	void doOK();
	void doCancel();
	void doSave();
	void doReset();
	void doCenter();
	void doSend();
	void poll_tracker_info();

protected:
	bool settingsDirty;
	void settings_changed();
	TrackerSettings settings;
	QTimer timer;
};


#endif //FTNOIR_TRACKER_HAT_DIALOG_H