#ifndef FTNOIR_TRACKER_HAT_DIALOG_H
#define FTNOIR_TRACKER_HAT_DIALOG_H

#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ftnoir_tracker_hat_settings.h"
#include "ftnoir_tracker_hat.h"
#include "ui_ftnoir_hatcontrols.h"

#include <QTimer>
#include <QMessageBox>

class TrackerControls: public QWidget, public ITrackerDialog
{
    Q_OBJECT

public:
	explicit TrackerControls();
    virtual ~TrackerControls() virt_override;

    void Initialize(QWidget *parent) virt_override;
    void registerTracker(ITracker *tracker) virt_override;
    void unRegisterTracker() virt_override;

private:
	Ui::UIHATControls ui;
    int pre_frame;
	FTNoIR_Tracker *theTracker;

protected slots:
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


    void set_rot_roll(int val)	 { settings.RollAxis = val;    settings_changed(); }
    void set_rot_pitch(int val)	 { settings.PitchAxis = val;   settings_changed(); }
    void set_rot_yaw(int val)	 { settings.YawAxis = val;     settings_changed(); }
    void set_acc_x(int val)		 { settings.XAxis = val;       settings_changed(); }
    void set_acc_y(int val)		 { settings.YAxis = val;       settings_changed(); }
    void set_acc_z(int val)		 { settings.ZAxis = val;       settings_changed(); }

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
