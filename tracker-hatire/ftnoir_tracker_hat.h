#pragma once

#include "thread.hpp"
#include "api/plugin-api.hpp"
#include "ftnoir_tracker_hat_settings.h"
#include "ftnoir_arduino_type.h"

#include <atomic>

#include <QObject>
#include <QByteArray>
#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSettings>

class hatire : public QObject, public ITracker
{
    Q_OBJECT

public:
    hatire();
    ~hatire();

    module_status start_tracker(QFrame*);
    void data(double *data);
    //void center();
    //bool notifyZeroed();
    void reset();
    void get_info( int *tps );
    void serial_info();
    void send_serial_command(const QByteArray& x);

    hatire_thread t;
private:
    TArduinoData ArduinoData, HAT;
    QByteArray Begin;
    QByteArray End;

    TrackerSettings s;

    int frame_cnt;

    std::atomic<int> CptError;

    static inline QByteArray to_latin1(const QString& str) { return str.toLatin1(); }
};

class hatire_metadata : public Metadata
{
    QString name() { return otr_tr("Hatire Arduino"); }
    QIcon icon() { return QIcon(":/images/hat.png"); }
};
