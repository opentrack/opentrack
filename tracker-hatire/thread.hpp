#pragma once

#include "ftnoir_arduino_type.h"
#include "ftnoir_tracker_hat_settings.h"

#include <QSerialPort>
#include <QByteArray>
#include <QThread>
#include <QMutex>

#include <QFile>
#include <QCoreApplication>

#include "compat/variance.hpp"
#include "compat/timer.hpp"

enum results
{
    result_ok,
    result_open_error,
    result_error,
};

//#define HATIRE_DEBUG_LOGFILE "c:/users/sthalik/test-random"

#ifdef HATIRE_DEBUG_LOGFILE
#   include <QFile>
#   include <QTimer>
#endif

struct serial_result
{
    serial_result() : code(result_ok) {}
    serial_result(results code, const QString& error) : error(error), code(code) {}

    QString error;
    results code;
};

class hatire_thread : public QThread
{
    Q_OBJECT

#ifdef HATIRE_DEBUG_LOGFILE
    using serial_t = QFile;
    QTimer read_timer;
#else
    using serial_t = QSerialPort;
#endif

    QByteArray data_read;
    serial_t com_port;
    TrackerSettings s;
    variance stat;
    Timer timer, throttle_timer;
    char buf[1024];

    void run() override;
    static inline QByteArray to_latin1(const QString& str) { return str.toLatin1(); }

    void serial_debug_info_str(const QString& str);

private slots:
    void on_serial_read();
    void teardown_serial();

    void sendcmd_impl(const QByteArray& cmd);
    void sendcmd_str_impl(const QString& str) { sendcmd(str.toLatin1()); }
    void serial_info_impl();
    serial_result init_serial_port_impl();

signals:
    void serial_debug_info(const QByteArray &MsgInfo);

    void sendcmd(const QByteArray& cmd);
    void sendcmd_str(const QString& cmd);
    void serial_info();
    serial_result init_serial_port();

public:
    void start();
    ~hatire_thread() override;
    hatire_thread();

    QByteArray& send_data_read_nolock();

    void Log(const QString& message);

    QMutex data_mtx;
};
