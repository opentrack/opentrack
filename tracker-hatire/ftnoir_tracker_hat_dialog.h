#pragma once

#include "api/plugin-api.hpp"
#include "compat/timer.hpp"
#include "ftnoir_tracker_hat_settings.h"
#include "ftnoir_tracker_hat.h"
#include "ui_ftnoir_hatcontrols.h"
#include <QObject>
#include <QWidget>
#include <QTimer>
#include <QMessageBox>
#include <QMetaType>

// Widget that has controls for FTNoIR protocol client-settings.
class dialog_hatire: public ITrackerDialog
{
    Q_OBJECT
public:
    explicit dialog_hatire();
    ~dialog_hatire() override;
    void Initialize(QWidget *parent) ; // unused
    void register_tracker(ITracker *tracker) override;
    void unregister_tracker() override;

private:
    Ui::UIHATControls ui;
    hatire *theTracker;
    Timer last_time;

public  slots:
    void WriteMsgInfo(const QByteArray &MsgInfo);

    void doOK();
    void doCancel();
    void doReset();
    //void doCenter();
    void doZero();
    void doSend();
    void poll_tracker_info();
    void doSerialInfo();

protected:
    TrackerSettings s;
    QTimer timer;

private slots:
    void on_lineSend_returnPressed();
};
