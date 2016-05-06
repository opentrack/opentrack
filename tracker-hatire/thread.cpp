#include "thread.hpp"
#include "opentrack-compat/sleep.hpp"
#include <utility>

#include <QTextStream>
#include <QTime>
#include <QDebug>

#include <cstring>

void hatire_thread::Log(const QString& message)
{
    // Drop out immediately if logging is off. Yes, there is still some overhead because of passing strings around for no reason.
    // that's unfortunate and I'll monitor the impact and see if it needs a more involved fix.
    if (!s.bEnableLogging) return;

    Diag flDiagnostics;

    if (flDiagnostics.open(QIODevice::ReadWrite | QIODevice::Append))
    {
        QTextStream out(&flDiagnostics);
        QString milliSeconds;
        milliSeconds = QString("%1").arg(QTime::currentTime().msec(), 3, 10, QChar('0'));
        // We have a file
        out << QTime::currentTime().toString() << "." << milliSeconds << ": " << message << "\r\n";
        flDiagnostics.close();
    }
}

void hatire_thread::start(const thread_settings& s_)
{
    s = s_;
    com_port.moveToThread(this);
#ifdef HATIRE_DEBUG_LOGFILE
    read_timer.moveToThread(this);
#endif
    QThread::start();
}

hatire_thread::~hatire_thread()
{
    quit();
    wait();
}

thread_settings hatire_thread::serial_settings_impl()
{
    return s;
}

hatire_thread::hatire_thread()
{
    data_read.reserve(65536);

    connect(this, &QThread::finished, this, &hatire_thread::teardown_serial);
    connect(this, &hatire_thread::update_serial_settings, this, &hatire_thread::update_serial_settings_impl, Qt::QueuedConnection);
    connect(this, &hatire_thread::init_serial_port, this, &hatire_thread::init_serial_port_impl, Qt::QueuedConnection);
    connect(this, &hatire_thread::serial_info, this, &hatire_thread::serial_info_impl, Qt::QueuedConnection);
    connect(this, &hatire_thread::sendcmd, this, &hatire_thread::sendcmd_impl, Qt::QueuedConnection);
    connect(this, &hatire_thread::serial_settings, this, &hatire_thread::serial_settings_impl, Qt::QueuedConnection);
}

void hatire_thread::teardown_serial()
{
    if (isRunning() && com_port.isOpen())
    {
        QByteArray msg;
        Log("Tracker shut down");
        com_port.write(s.sCmdStop);
        if (!com_port.waitForBytesWritten(1000))
        {
            emit serial_debug_info("TimeOut in writing CMD");
        }
        else
        {
            msg.append("\r\n");
            msg.append("SEND '");
            msg.append(s.sCmdStop);
            msg.append("'\r\n");
        }
        emit serial_debug_info(msg);

        disconnect(&com_port, SIGNAL(readyRead()), nullptr, nullptr);
        com_port.close();
    }
}

void hatire_thread::run()
{
#ifdef HATIRE_DEBUG_LOGFILE
    com_port.setFileName(HATIRE_DEBUG_LOGFILE);
    com_port.open(QIODevice::ReadOnly);

    connect(&read_timer, &QTimer::timeout, this, &hatire_thread::on_serial_read, Qt::DirectConnection);
    read_timer.start(16);
#else
    connect(&com_port, &serial_t::readyRead, this, &hatire_thread::on_serial_read, Qt::DirectConnection);
#endif
    (void) exec();
}

void hatire_thread::update_serial_settings_impl(const thread_settings &s_)
{
    s = s_;
}

serial_result hatire_thread::init_serial_port_impl()
{
#ifndef HATIRE_DEBUG_LOGFILE
    com_port.setPortName(s.sSerialPortName);

    if (com_port.open(QIODevice::ReadWrite))
    {
        Log("Port Open");
        if (
            com_port.setBaudRate((QSerialPort::BaudRate)s.iBaudRate)
            && com_port.setDataBits((QSerialPort::DataBits)s.iDataBits)
            && com_port.setParity((QSerialPort::Parity)s.iParity)
            && com_port.setStopBits((QSerialPort::StopBits)s.iStopBits)
            && com_port.setFlowControl((QSerialPort::FlowControl)s.iFlowControl)
            && com_port.clear(QSerialPort::AllDirections)
            && com_port.setDataErrorPolicy(QSerialPort::IgnorePolicy)
           )
        {
            Log("Port Parameters set");
            qDebug()  << QTime::currentTime()  << " HAT OPEN   on " << com_port.portName() <<  com_port.baudRate() <<  com_port.dataBits() <<  com_port.parity() <<  com_port.stopBits() <<  com_port.flowControl();

            if (com_port.flowControl() == QSerialPort::HardwareControl)
            {
                // Raise DTR
                Log("Raising DTR");
                if (!com_port.setDataTerminalReady(true))
                    Log("Couldn't set DTR");

                // Raise RTS/CTS
                Log("Raising RTS");
                if (!com_port.setRequestToSend(true))
                    Log("Couldn't set RTS");
            }
            // Wait init arduino sequence
            for (int i = 1; i <=s.iDelayInit;  i+=50) {
                if (com_port.waitForReadyRead(50)) break;
            }
            Log("Waiting on init");
            qDebug()  << QTime::currentTime()  << " HAT send INIT ";
            sendcmd(s.sCmdInit);
            // Wait init MPU sequence
            for (int i = 1; i <=s.iDelayStart;  i+=50) {
                if (com_port.waitForReadyRead(50)) break;
            }
            // Send  START cmd to IMU
            qDebug()  << QTime::currentTime()  << " HAT send START ";
            sendcmd(s.sCmdStart);

            // Wait start MPU sequence
            for (int i = 1; i <=s.iDelaySeq;  i+=50) {
                if (com_port.waitForReadyRead(50)) break;
            }
            Log("Port setup, waiting for HAT frames to process");
            qDebug()  << QTime::currentTime()  << " HAT wait MPU ";

            return serial_result();
        }
        else
        {
            return serial_result(result_error, com_port.errorString());
        }
    }
    else
        return serial_result(result_open_error, com_port.errorString());
#else
    return serial_result();
#endif
}

// Info SerialPort
void hatire_thread::serial_info_impl()
{
#ifndef HATIRE_DEBUG_LOGFILE
    QByteArray msg;

    if (com_port.isOpen())
    {
        msg.append("\r\n");
        msg.append(com_port.portName());
        msg.append("\r\n");
        msg.append("BAUDRATE :");
        msg.append(QString::number(com_port.baudRate()));
        msg.append("\r\n");
        msg.append("DataBits :");
        msg.append(QString::number(com_port.dataBits()));
        msg.append("\r\n");
        msg.append("Parity :");

        switch (com_port.parity())
        {
        case 0:  msg.append("No parity");
            break;
        case 2:  msg.append("Even parity");
            break;
        case 3:  msg.append("Odd parity");
            break;
        case 4:  msg.append("Space parity");
            break;
        case 5:  msg.append("Mark parity");
            break;
        default:  msg.append("Unknown parity");
            break;
        }

        msg.append("\r\n");
        msg.append("Stop Bits :");

        switch (com_port.stopBits())
        {
        msg.append(QString::number(com_port.stopBits()));
        case 1:  msg.append("1 stop bit.");
            break;
        case 2:  msg.append("2 stop bits.");
            break;
        case 3:  msg.append("1.5 stop bits.");
            break;
        default:  msg.append("Unknown number of stop bit.");
            break;
        }

        msg.append("\r\n");
        msg.append("Flow Control :");
        switch (com_port.flowControl())
        {
        case 0:  msg.append("No flow control");
            break;
        case 1:  msg.append("Hardware flow control (RTS/CTS)");
            break;
        case 2:  msg.append("Software flow control (XON/XOFF)");
            break;
        default:  msg.append("Unknown flow control");
            break;
        }

        emit serial_debug_info(msg);
    }
#endif
}

#ifdef __GNUC__
#   define unused(t, i) t __attribute__((unused)) i
#else
#   define unused(t, i) t i
#endif

//send command  to Arduino


void hatire_thread::on_serial_read()
{
    constexpr int hz = 90;
    constexpr int ms = 1000/hz;

    {
        QMutexLocker lck(&data_mtx);
#ifndef HATIRE_DEBUG_LOGFILE
        data_read += com_port.readAll();
#else
        QByteArray tmp = com_port.read(30);
        data_read += tmp;
        if (tmp.length() == 0)
        {
            qDebug() << "eof";
            read_timer.stop();
        }
#endif
    }
    // qt can fire QSerialPort::readyRead() needlessly, causing a busy loop.
    // see https://github.com/opentrack/opentrack/issues/327#issuecomment-207941003
    portable::sleep(ms);
}

QByteArray& hatire_thread::send_data_read_nolock(bool& ret)
{
    constexpr int packet_len = 30;
    constexpr int cnt = 4;
    constexpr int len = cnt * packet_len;

    if (data_read.length() < len)
    {
        // we're requesting more than packet length to help resync the stream if needed
        ret = false;
    }
    else
        ret = true;

    return data_read;
}
