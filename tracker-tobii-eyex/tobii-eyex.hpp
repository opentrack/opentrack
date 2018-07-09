#pragma once

/* Copyright (c) 2016 Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "tobii-settings.hpp"

#include "api/plugin-api.hpp"
#include "options/options.hpp"
using namespace options;
#include "compat/timer.hpp"

#include <EyeX.h>

#include <functional>
#include <atomic>

#include <QObject>
#include <QMutex>

//using real = TX_REAL;
using real = double;

struct state
{
    real display_res_x = -1, display_res_y = -1;
    real px = -1, py = -1;
    real last_timestamp = -1;
    bool fresh = false;

    state();
    bool is_valid() const { return !(display_res_x < 0 || px < 0); }
};

class tobii_eyex_tracker : public TR, public ITracker
{
    Q_OBJECT

public:
    tobii_eyex_tracker();
    ~tobii_eyex_tracker() override;
    module_status start_tracker(QFrame*) override;
    void data(double *data) override;
    bool center() override;
private:
    static constexpr inline const char* const client_id = "opentrack-tobii-eyex";

    static void call_tx_deinit();

    static bool register_state_snapshot(TX_CONTEXTHANDLE ctx, TX_HANDLE* state_snapshot_ptr);
    static std::atomic_flag atexit_done;
    static void TX_CALLCONVENTION state_change_handler(TX_CONNECTIONSTATE state, TX_USERPARAM param);
    static void TX_CALLCONVENTION event_handler(TX_CONSTHANDLE async_data_handle, TX_USERPARAM param);
    void gaze_data_handler(TX_HANDLE gaze_data_handle);
    static void TX_CALLCONVENTION snapshot_committed_handler(TX_CONSTHANDLE async_data_handle, TX_USERPARAM param);
    static void TX_CALLCONVENTION display_state_handler(TX_CONSTHANDLE async_data_handle, TX_USERPARAM param);
    void process_display_state(TX_HANDLE display_state_handle);

    real gain(real x);

    state dev_state;
    real yaw = 0, pitch = 0;

    TX_CONTEXTHANDLE ctx = TX_EMPTY_HANDLE;
    TX_TICKET state_changed_ticket = TX_INVALID_TICKET;
    TX_TICKET event_handler_ticket = TX_INVALID_TICKET;
    TX_HANDLE state_snapshot = TX_EMPTY_HANDLE;
    TX_HANDLE display_state = TX_EMPTY_HANDLE;

    QMutex global_state_mtx;
    settings s;
    Timer t;

    std::atomic<bool> do_center = false;
};

class tobii_eyex_metadata : public Metadata
{
    Q_OBJECT

    QString name() override { return QString("Tobii EyeX"); }
    QIcon icon() override { return QIcon(":/images/tobii-eyex-logo.png"); }
};

