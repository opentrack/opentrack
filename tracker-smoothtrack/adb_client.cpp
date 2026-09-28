/* Copyright (c) 2026 CyberoniOntoni
 *
 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "adb_client.h"

#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QHostAddress>
#include <QObject>
#include <QStandardPaths>
#include <QTcpSocket>
#include <QDebug>
#include <QThread>
#include <QRegularExpression>

namespace {

#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
constexpr auto SplitSkipEmpty = Qt::SkipEmptyParts;
#else
constexpr auto SplitSkipEmpty = QString::SkipEmptyParts;
#endif

QStringList with_serial(const QString& serial, QStringList args)
{
    if (!serial.isEmpty())
        args = QStringList{"-s", serial} + args;
    return args;
}

bool adb_server_listening()
{
    QTcpSocket sock;
    sock.connectToHost(QHostAddress::LocalHost, 5037);
    const bool ok = sock.waitForConnected(200);
    sock.abort();
    return ok;
}

QString abi_to_suffix(const QString& abi)
{
    if (abi.contains("arm64") || abi.contains("aarch64"))
        return QStringLiteral("arm64");
    if (abi.contains("v7") || abi.contains("armeabi"))
        return QStringLiteral("armv7");
    return QString();
}

bool run_adb_cmd(const QString& adb_path, const QStringList& args, int timeout_ms,
                 QString* stdout_str = nullptr, QString* stderr_str = nullptr, int* exit_code = nullptr)
{
    if (adb_path.isEmpty() || !QFileInfo::exists(adb_path))
    {
        if (stderr_str)
            *stderr_str = "ADB executable not found";
        return false;
    }

    QProcess proc;
    QElapsedTimer timer;
    timer.start();
    proc.start(adb_path, args);

    if (!proc.waitForStarted(qMin(timeout_ms, 5000)))
    {
        proc.kill();
        proc.waitForFinished(200);
        if (stderr_str)
            *stderr_str = QString("Failed to start '%1': %2").arg(adb_path, proc.errorString());
        if (exit_code)
            *exit_code = -1;
        return false;
    }

    const int remaining = qMax(0, timeout_ms - static_cast<int>(timer.elapsed()));
    if (!proc.waitForFinished(remaining))
    {
        proc.kill();
        proc.waitForFinished(200);
        if (stderr_str)
            *stderr_str = "Process timed out";
        if (exit_code)
            *exit_code = -1;
        return false;
    }

    if (stdout_str)
        *stdout_str = QString::fromUtf8(proc.readAllStandardOutput());
    if (stderr_str)
        *stderr_str = QString::fromUtf8(proc.readAllStandardError());
    if (exit_code)
        *exit_code = proc.exitCode();

    return proc.exitCode() == 0;
}

void kill_device_relay(const QString& adb_path, const QString& serial)
{
    static const QString kKill =
        QStringLiteral("pkill -f st-relay || killall st-relay || "
                       "kill $(pidof st-relay) || kill $(pidof /data/local/tmp/st-relay) || true");
    run_adb_cmd(adb_path, with_serial(serial, {"shell", "sh", "-c", kKill}),
                adb_client::QUICK_TIMEOUT_MS);
}

QString collect_qprocess_output(QProcess* proc)
{
    if (!proc)
        return QString();
    const QString err = QString::fromUtf8(proc->readAllStandardError()).trimmed();
    const QString out = QString::fromUtf8(proc->readAllStandardOutput()).trimmed();
    if (err.isEmpty())
        return out;
    if (out.isEmpty())
        return err;
    return err + QLatin1Char('\n') + out;
}

} // anonymous namespace

QString adb_client::find_adb(const QString& user_hint)
{
    auto try_exe = [](const QString& path) -> QString {
        if (!path.isEmpty() && QFileInfo::exists(path) && !QFileInfo(path).isDir())
            return QDir::toNativeSeparators(path);
        return QString();
    };

    auto try_root = [&](const QString& root) -> QString {
        if (root.isEmpty())
            return QString();
        const QFileInfo fi(root);
        if (fi.exists() && !fi.isDir())
            return QDir::toNativeSeparators(root);
        QString found = try_exe(root + "/adb.exe");
        if (!found.isEmpty())
            return found;
        return try_exe(root + "/adb");
    };

    const QString hint = user_hint.trimmed();
    if (!hint.isEmpty())
    {
        const QString found = try_root(hint);
        if (!found.isEmpty())
            return found;
    }

    {
        const QString found = try_root(QCoreApplication::applicationDirPath());
        if (!found.isEmpty())
            return found;
    }

    QString found = QStandardPaths::findExecutable("adb");
    if (found.isEmpty())
        found = QStandardPaths::findExecutable("adb.exe");
    if (!found.isEmpty())
        return QDir::toNativeSeparators(found);

    return QString();
}

QList<adb_client::device_info> adb_client::list_devices(const QString& adb_path, QString* error_msg)
{
    QList<device_info> result;
    if (adb_path.isEmpty() || !QFileInfo::exists(adb_path))
    {
        if (error_msg)
        {
            if (!adb_path.isEmpty())
                *error_msg = QObject::tr("ADB executable not found at '%1'.\n"
                                         "Please verify the path in SmoothTrack settings or install Android Platform Tools.")
                                 .arg(adb_path);
            else
                *error_msg = QObject::tr("ADB executable not found.\n"
                                         "Please install Android Platform Tools, place adb in the OpenTrack directory or on system PATH, "
                                         "or specify its location in SmoothTrack settings.");
        }
        return result;
    }

    QString output, err_str;
    int exit_code = 0;
    if (!run_adb_cmd(adb_path, QStringList{"devices", "-l"}, DEFAULT_TIMEOUT_MS, &output, &err_str, &exit_code))
    {
        if (error_msg)
        {
            if (err_str == "Process timed out")
                *error_msg = QObject::tr("ADB timed out while querying connected devices.\n"
                                         "The ADB server may be unresponsive. Try running 'adb kill-server' in a terminal or reconnecting the USB cable.");
            else
                *error_msg = QObject::tr("ADB command failed (%1): %2")
                                 .arg(exit_code)
                                 .arg(err_str.trimmed().isEmpty() ? "Unknown error" : err_str.trimmed());
        }
        return result;
    }

    const QStringList lines = output.split(QRegularExpression("[\r\n]+"), SplitSkipEmpty);

    for (const QString& line : lines)
    {
        const QString trimmed = line.trimmed();
        if (trimmed.startsWith("List of devices") || trimmed.startsWith("*"))
            continue;

        const QStringList tokens = trimmed.split(QRegularExpression("\\s+"), SplitSkipEmpty);
        if (tokens.size() >= 2)
        {
            device_info dev;
            dev.serial = tokens[0];
            dev.status = tokens[1];

            for (const QString& token : tokens)
            {
                if (token.startsWith("model:"))
                    dev.model = token.mid(6);
            }
            result.append(dev);
        }
    }

    return result;
}

bool adb_client::check_device(const QString& adb_path, QString* error_msg, QString* chosen_serial)
{
    const QList<device_info> devices = list_devices(adb_path, error_msg);
    if (devices.isEmpty())
    {
        if (error_msg && error_msg->isEmpty())
            *error_msg = QObject::tr("No Android device detected over USB.\n\n"
                                     "Troubleshooting steps:\n"
                                     "1. Connect your Android phone to the PC via USB cable.\n"
                                     "2. Enable Developer Options on your phone:\n"
                                     "   Settings -> About Phone -> tap 'Build number' 7 times.\n"
                                     "3. Enable USB Debugging:\n"
                                     "   Settings -> Developer Options -> turn ON 'USB debugging'.\n"
                                     "4. Ensure USB connection mode is 'File Transfer' / 'MTP' (not 'Charge only').\n"
                                     "5. Check your phone screen for an authorization prompt.");
        return false;
    }

    for (const device_info& dev : devices)
    {
        if (dev.status == "device")
        {
            if (chosen_serial)
                *chosen_serial = dev.serial;
            return true;
        }
    }

    for (const device_info& dev : devices)
    {
        if (dev.status == "unauthorized")
        {
            if (error_msg)
                *error_msg = QObject::tr("Android device '%1' is unauthorized.\n\n"
                                         "To authorize:\n"
                                         "1. Unlock your phone screen.\n"
                                         "2. Look for the prompt: 'Allow USB debugging?'.\n"
                                         "3. Check 'Always allow from this computer' and tap 'Allow'.\n"
                                         "4. If no prompt appears, reconnect the USB cable.")
                                 .arg(dev.serial);
            return false;
        }
        if (dev.status == "offline")
        {
            if (error_msg)
                *error_msg = QObject::tr("Android device '%1' is offline.\n\n"
                                         "1. Reconnect the USB cable.\n"
                                         "2. Toggle 'USB debugging' off and on in Developer Options.")
                                 .arg(dev.serial);
            return false;
        }
    }

    if (error_msg)
        *error_msg = QObject::tr("No ready Android devices found (all connected devices are offline or unauthorized).");
    return false;
}

QString adb_client::get_device_abi(const QString& adb_path, const QString& serial)
{
    QString output;
    if (run_adb_cmd(adb_path, with_serial(serial, {"shell", "getprop", "ro.product.cpu.abi"}),
                    QUICK_TIMEOUT_MS, &output))
    {
        const QString abi = output.trimmed();
        if (!abi.isEmpty())
            return abi;
    }

    return QString();
}

QString adb_client::find_relay_binary(const QString& abi)
{
    const QString suffix = abi_to_suffix(abi);
    if (suffix.isEmpty())
        return QString();

    const QString app_dir = QCoreApplication::applicationDirPath();
    const QString filename = QStringLiteral("st-relay-") + suffix;
    const QStringList candidates = {
        app_dir + "/modules/android/" + filename,
        app_dir + "/../libexec/opentrack/android/" + filename,
        app_dir + "/../Plugins/android/" + filename,
    };

    for (const QString& candidate : candidates)
    {
        if (!candidate.isEmpty() && QFileInfo::exists(candidate) && !QFileInfo(candidate).isDir())
            return QDir::toNativeSeparators(candidate);
    }

    return QString();
}

bool adb_client::setup_reverse(const QString& adb_path, int host_port, int device_port,
                               const QString& serial, QString* error_msg)
{
    QString err_str;
    if (!run_adb_cmd(adb_path,
                     with_serial(serial, {"reverse", QString("tcp:%1").arg(device_port),
                                          QString("tcp:%1").arg(host_port)}),
                     DEFAULT_TIMEOUT_MS, nullptr, &err_str))
    {
        if (error_msg)
        {
            const QString err = err_str.trimmed();
            *error_msg = QObject::tr("Failed to setup ADB reverse port forwarding (tcp:%1 -> tcp:%2): %3\n"
                                     "Ensure the port is not already bound by another process.")
                             .arg(device_port)
                             .arg(host_port)
                             .arg(err.isEmpty() ? "Unknown error or timeout" : err);
        }
        return false;
    }
    return true;
}

bool adb_client::remove_reverse(const QString& adb_path, int device_port, const QString& serial)
{
    return run_adb_cmd(adb_path,
                       with_serial(serial, {"reverse", "--remove", QString("tcp:%1").arg(device_port)}),
                       QUICK_TIMEOUT_MS);
}

adb_client::adb_client() = default;

adb_client::~adb_client()
{
    stop();
}

bool adb_client::start(const QString& adb_path, int udp_port, int tcp_port, QString* error_msg)
{
    stop();

    active_adb = adb_path;
    last_relay_stderr.clear();

    const bool server_already_up = adb_server_listening();
    QString err_str;
    if (!run_adb_cmd(active_adb, {"start-server"}, START_SERVER_TIMEOUT_MS, nullptr, &err_str))
    {
        started_adb_server = !server_already_up;
        if (error_msg)
            *error_msg = QObject::tr("Failed to start ADB server: %1")
                             .arg(err_str.trimmed().isEmpty() ? "Unknown error" : err_str.trimmed());
        stop();
        return false;
    }
    started_adb_server = !server_already_up;

    if (!check_device(active_adb, error_msg, &active_serial))
    {
        stop();
        return false;
    }

    if (!setup_reverse(active_adb, tcp_port, tcp_port, active_serial, error_msg))
    {
        stop();
        return false;
    }
    active_port = tcp_port;
    reverse_installed = true;

    const QString abi = get_device_abi(active_adb, active_serial);
    const QString suffix = abi_to_suffix(abi);
    if (suffix.isEmpty())
    {
        if (error_msg)
            *error_msg = abi.isEmpty()
                             ? QObject::tr("Could not determine Android ABI (need armv7 or arm64).")
                             : QObject::tr("Unsupported Android ABI '%1' (need armv7 or arm64).").arg(abi);
        stop();
        return false;
    }

    const QString relay_bin = find_relay_binary(abi);
    if (relay_bin.isEmpty())
    {
        if (error_msg)
            *error_msg = QObject::tr("Relay binary (st-relay-%1) not found.\n"
                                     "Looked in modules/android, libexec/opentrack/android, and Plugins/android.")
                             .arg(suffix);
        stop();
        return false;
    }

    QString push_err;
    if (!run_adb_cmd(active_adb,
                     with_serial(active_serial, {"push", relay_bin, "/data/local/tmp/st-relay"}),
                     PUSH_TIMEOUT_MS, nullptr, &push_err))
    {
        if (error_msg)
            *error_msg = QObject::tr("Failed to push relay binary to Android device: %1")
                             .arg(push_err.trimmed().isEmpty() ? "Transfer error or timeout" : push_err.trimmed());
        stop();
        return false;
    }

    run_adb_cmd(active_adb,
                with_serial(active_serial, {"shell", "chmod", "755", "/data/local/tmp/st-relay"}),
                QUICK_TIMEOUT_MS);

    kill_device_relay(active_adb, active_serial);
    QThread::msleep(100);

    relay_proc = std::make_unique<QProcess>();
    relay_proc->start(active_adb,
                      with_serial(active_serial,
                                  {"shell", "/data/local/tmp/st-relay",
                                   QString::number(udp_port), QString::number(tcp_port)}));
    if (!relay_proc->waitForStarted(DEFAULT_TIMEOUT_MS))
    {
        if (error_msg)
            *error_msg = QObject::tr("Failed to start '%1': %2")
                             .arg(active_adb, relay_proc->errorString());
        relay_proc.reset();
        stop();
        return false;
    }

    if (relay_proc->waitForFinished(400))
    {
        last_relay_stderr = collect_qprocess_output(relay_proc.get());
        if (error_msg)
        {
            *error_msg = last_relay_stderr.isEmpty()
                             ? QObject::tr("st-relay exited immediately (code %1).")
                                   .arg(relay_proc->exitCode())
                             : last_relay_stderr;
        }
        stop();
        return false;
    }

    return true;
}

void adb_client::stop()
{
    if (relay_proc)
    {
        if (relay_proc->state() != QProcess::NotRunning)
        {
            relay_proc->terminate();
            if (!relay_proc->waitForFinished(500))
            {
                relay_proc->kill();
                relay_proc->waitForFinished(200);
            }
        }
        relay_proc.reset();
    }

    if (reverse_installed && active_port > 0 && !active_adb.isEmpty() && QFileInfo::exists(active_adb))
    {
        kill_device_relay(active_adb, active_serial);
        remove_reverse(active_adb, active_port, active_serial);
    }

    if (started_adb_server && !active_adb.isEmpty() && QFileInfo::exists(active_adb))
        run_adb_cmd(active_adb, {"kill-server"}, DEFAULT_TIMEOUT_MS);

    reverse_installed = false;
    started_adb_server = false;
    active_adb.clear();
    active_serial.clear();
    active_port = 0;
}

bool adb_client::is_running() const
{
    return relay_proc && relay_proc->state() == QProcess::Running;
}

QString adb_client::relay_stderr()
{
    const QString chunk = collect_qprocess_output(relay_proc.get());
    if (!chunk.isEmpty())
        last_relay_stderr = chunk;
    return last_relay_stderr;
}
