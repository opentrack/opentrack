#pragma once

#include <QSerialPort>
#include <QByteArray>
#include <QThread>
#include <QMutex>

#include <QFile>
#include <QCoreApplication>

#include "opentrack/variance.hpp"
#include "opentrack-compat/timer.hpp"

enum results
{
    result_ok,
    result_open_error,
    result_error,
};

//#define HATIRE_DEBUG_LOGFILE "d:/putty-hatire.log"

#ifdef HATIRE_DEBUG_LOGFILE
#   include <QFile>
#   include <QTimer>
#endif

#ifdef __GNUC__
#   define unused(t, i) t __attribute__((unused)) i
#else
#   define unused(t, i) t i
#endif

struct thread_settings
{
    QByteArray  sCmdStart;
    QByteArray  sCmdStop;
    QByteArray  sCmdInit;
    QByteArray  sCmdReset;
    QByteArray  sCmdCenter;
    QByteArray  sCmdZero;

    QString sSerialPortName;
    QSerialPort::BaudRate iBaudRate;
    QSerialPort::DataBits iDataBits;
    QSerialPort::Parity iParity;
    QSerialPort::StopBits iStopBits;
    QSerialPort::FlowControl iFlowControl;

    int iDelayInit;
    int iDelayStart;
    int iDelaySeq;
    bool bBigEndian;
    volatile bool bEnableLogging;

    thread_settings() :
        iBaudRate(QSerialPort::UnknownBaud),
        iDataBits(QSerialPort::UnknownDataBits),
        iParity(QSerialPort::UnknownParity),
        iStopBits(QSerialPort::UnknownStopBits),
        iFlowControl(QSerialPort::UnknownFlowControl),
        iDelayInit(0),
        iDelayStart(0),
        iDelaySeq(0),
        bBigEndian(false),
        bEnableLogging(false)
    {
    }
};

#include <QMetaType>

Q_DECLARE_METATYPE(thread_settings)

struct serial_result
{
    serial_result() : code(result_ok) {}
    serial_result(results code, const QString& error) : error(error), code(code) {}

    QString error;
    results code;
};

struct Diag : public QFile
{
    Diag()
    {
        setFileName(QCoreApplication::applicationDirPath() + "/HATDiagnostics.txt");
    }
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
    thread_settings s;
    variance stat;
    Timer timer, throttle_timer;

    void run() override;

private slots:
    void on_serial_read();
    void teardown_serial();

    void sendcmd_impl(unused(const QByteArray, &cmd));
    void serial_info_impl();
    serial_result init_serial_port_impl();
    void update_serial_settings_impl(const thread_settings& s);
    thread_settings serial_settings_impl();

signals:
    void serial_debug_info(const QByteArray &MsgInfo);

    void sendcmd(const QByteArray& cmd);
    void serial_info();
    serial_result init_serial_port();
    void update_serial_settings(const thread_settings& s);
    thread_settings serial_settings();

public:
    void start(const thread_settings &s_);
    ~hatire_thread() override;
    hatire_thread();

    QByteArray& send_data_read_nolock(bool& ret);

    void Log(const QString& message);

    QMutex data_mtx;
};
