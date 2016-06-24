#include "thread.hpp"
#include "opentrack-compat/sleep.hpp"
#include <utility>

#include <QTextStream>
#include <QTime>
#include <QDebug>

#include <cstring>

void hatire_thread::sendcmd_impl(const QByteArray &cmd)
{
#ifndef HATIRE_DEBUG_LOGFILE
    QByteArray Msg;

    if (cmd.length() > 0)
    {
        if (com_port.isOpen())
        {
            QString logMess;
            logMess.append("SEND '");
            logMess.append(cmd);
            logMess.append("'");
            Log(logMess);
            com_port.write(cmd);
            if (!com_port.waitForBytesWritten(1000)) {
                emit serial_debug_info("TimeOut in writing CMD");
            } else
            {
                Msg.append("\r\n");
                Msg.append("SEND '");
                Msg.append(cmd);
                Msg.append("'\r\n");
            }
#if 0 // WaitForReadyRead isn't working well and there are some reports of it being a win32 issue. We can live without it anyway
            if  ( !com_port.waitForReadyRead(1000)) {
                emit serial_debug_info("TimeOut in response to CMD") ;
            } else {
                emit serial_debug_info(Msg);
            }
#else
            emit serial_debug_info(Msg);
#endif
        } else {
            emit serial_debug_info("ComPort not open")  ;
        }
    }
#endif
}

struct Diag final : public QFile
{
    Diag()
    {
        setFileName(QCoreApplication::applicationDirPath() + "/HATDiagnostics.txt");
    }
};

void hatire_thread::Log(const QString& message)
{
    // Drop out immediately if logging is off. Yes, there is still some overhead because of passing strings around for no reason.
    // that's unfortunate and I'll monitor the impact and see if it needs a more involved fix.
    if (!s.EnableLogging) return;

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

void hatire_thread::start()
{
    QThread::start();
}

hatire_thread::~hatire_thread()
{
    quit();
    wait();
}

hatire_thread::hatire_thread()
{
    data_read.reserve(65536);

    connect(this, &QThread::finished, this, &hatire_thread::teardown_serial, Qt::DirectConnection);
    connect(this, &hatire_thread::init_serial_port, this, &hatire_thread::init_serial_port_impl, Qt::QueuedConnection);
    connect(this, &hatire_thread::serial_info, this, &hatire_thread::serial_info_impl, Qt::QueuedConnection);
    connect(this, &hatire_thread::sendcmd, this, &hatire_thread::sendcmd_impl, Qt::QueuedConnection);
    connect(this, &hatire_thread::sendcmd_str, this, &hatire_thread::sendcmd_str_impl, Qt::QueuedConnection);

    //com_port.moveToThread(this);
#ifdef HATIRE_DEBUG_LOGFILE
    read_timer.moveToThread(this);
#endif

    connect(&com_port, &serial_t::readyRead, this, &hatire_thread::on_serial_read, Qt::DirectConnection);
}

void hatire_thread::teardown_serial()
{
    if (isRunning() && com_port.isOpen())
    {
        QByteArray msg;
        Log("Tracker shut down");
        com_port.write(to_latin1(s.CmdStop));
        if (!com_port.waitForBytesWritten(1000))
        {
            emit serial_debug_info("TimeOut in writing CMD");
        }
        else
        {
            msg.append("\r\n");
            msg.append("SEND '");
            msg.append(s.CmdStop);
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

    read_timer.start(10);
    connect(&read_timer, &QTimer::timeout, this, &hatire_thread::on_serial_read, Qt::DirectConnection);
#endif
    (void) exec();

#ifdef HATIRE_DEBUG_LOGFILE
    read_timer.stop();
#endif
}

serial_result hatire_thread::init_serial_port_impl()
{
#ifndef HATIRE_DEBUG_LOGFILE
    Log("Setting serial port name");
    com_port.setPortName(s.QSerialPortName);

    Log("Opening serial port");
    if (com_port.open(QIODevice::ReadWrite))
    {
        Log("Port Open");
        if (
            com_port.setBaudRate((QSerialPort::BaudRate)s.pBaudRate)
            && com_port.setDataBits((QSerialPort::DataBits)s.pDataBits)
            && com_port.setParity((QSerialPort::Parity)s.pParity)
            && com_port.setStopBits((QSerialPort::StopBits)s.pStopBits)
            && com_port.setFlowControl((QSerialPort::FlowControl)s.pFlowControl)
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
            for (int i = 1; i <= s.DelayInit; i+=50)
            {
                if (com_port.waitForReadyRead(50)) break;
            }
            Log("Waiting on init");
            qDebug()  << QTime::currentTime()  << " HAT send INIT ";
            sendcmd_str(s.CmdInit);
            // Wait init MPU sequence
            for (int i = 1; i <= s.DelayStart; i+=50)
            {
                if (com_port.waitForReadyRead(50)) break;
            }
            // Send  START cmd to IMU
            qDebug()  << QTime::currentTime()  << " HAT send START ";
            sendcmd_str(s.CmdStart);

            // Wait start MPU sequence
            for (int i = 1; i <=s.DelaySeq; i+=50)
            {
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

void hatire_thread::on_serial_read()
{
    {
        QMutexLocker lck(&data_mtx);
#ifndef HATIRE_DEBUG_LOGFILE
        data_read += com_port.readAll();
#else
        QByteArray tmp = com_port.read(30);
        data_read += tmp;
        if (tmp.length() == 0 && read_timer.isActive())
        {
            qDebug() << "eof";
            read_timer.stop();
        }
#endif
    }

    stat.input(timer.elapsed_ms());
    timer.start();

    if (throttle_timer.elapsed_ms() >= 3000)
    {
        throttle_timer.start();
        qDebug() << "stat:" << "avg" << stat.avg() << "stddev" << stat.stddev();
    }

    if (s.serial_bug_workaround)
    {
        // qt can fire QSerialPort::readyRead() needlessly, causing a busy loop.
        // see https://github.com/opentrack/opentrack/issues/327#issuecomment-207941003
        constexpr int hz = 90;
        constexpr int ms = 1000/hz;
        portable::sleep(ms);
    }
}

QByteArray& hatire_thread::send_data_read_nolock()
{
    return data_read;
}
