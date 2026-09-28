/* Copyright (c) 2026 CyberoniOntoni
 *
 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QString>
#include <QStringList>
#include <QProcess>
#include <QList>
#include <memory>

class adb_client
{
public:
    static constexpr int START_SERVER_TIMEOUT_MS = 20000;
    static constexpr int PUSH_TIMEOUT_MS = 15000;
    static constexpr int DEFAULT_TIMEOUT_MS = 5000;
    static constexpr int QUICK_TIMEOUT_MS = 1500;

    struct device_info
    {
        QString serial;
        QString status; // "device", "unauthorized", "offline"
        QString model;
    };

    static QString find_adb(const QString& user_hint = QString());
    static QList<device_info> list_devices(const QString& adb_path, QString* error_msg = nullptr);
    static bool check_device(const QString& adb_path, QString* error_msg, QString* chosen_serial = nullptr);
    static QString get_device_abi(const QString& adb_path, const QString& serial = QString());
    static QString find_relay_binary(const QString& abi);
    static bool setup_reverse(const QString& adb_path, int host_port, int device_port,
                              const QString& serial = QString(), QString* error_msg = nullptr);
    static bool remove_reverse(const QString& adb_path, int device_port,
                                const QString& serial = QString());

    adb_client();
    ~adb_client();

    bool start(const QString& adb_path, int udp_port, int tcp_port, QString* error_msg);
    void stop();
    bool is_running() const;
    QString relay_stderr();

private:
    QString active_adb;
    QString active_serial;
    int active_port{0};
    bool reverse_installed{false};
    bool started_adb_server{false};
    QString last_relay_stderr;
    std::unique_ptr<QProcess> relay_proc;
};
