#pragma once

/* Copyright (c) 2016 Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include <EyeX.h>

#include "ui_tobii-eyex-dialog.h"
#include "opentrack/plugin-api.hpp"
#include "opentrack-compat/options.hpp"
using namespace options;
#include "opentrack-compat/timer.hpp"
#include "spline-widget/functionconfig.h"
#include "spline-widget/qfunctionconfigurator.h"

#include <atomic>
#include <QObject>
#include <QMutex>

enum tobii_mode
{
    tobii_relative,
    tobii_absolute,
};

struct rel_settings : public QObject, public opts
{
    using s = slider_value;
    value<slider_value> speed, dz_end_pt, expt_val, log_base;
    rel_settings();
private:
    // linear coefficient to be the same as exponent
    Map spline;
};

struct settings : public opts
{
    value<tobii_mode> mode;
    settings() :
        opts("tobii-eyex"),
        mode(b, "mode", tobii_relative)
    {}
};

class tobii_eyex_tracker : public ITracker
{
public:
    tobii_eyex_tracker();
    ~tobii_eyex_tracker() override;
    void start_tracker(QFrame *) override;
    void data(double *data) override;
    bool center() override
    {
        do_center = true;
        return true;
    }
private:
    static constexpr const char* client_id = "opentrack-tobii-eyex";

    static void call_tx_deinit();

    static bool register_state_snapshot(TX_CONTEXTHANDLE ctx, TX_HANDLE* state_snapshot_ptr);
    static std::atomic_flag atexit_done;
    static void TX_CALLCONVENTION connection_state_change_handler(TX_CONNECTIONSTATE state, TX_USERPARAM param);
    static void TX_CALLCONVENTION event_handler(TX_CONSTHANDLE async_data_handle, TX_USERPARAM param);
    void gaze_data_handler(TX_HANDLE gaze_data_handle);
    static void TX_CALLCONVENTION snapshot_committed_handler(TX_CONSTHANDLE async_data_handle, TX_USERPARAM param);
    static void TX_CALLCONVENTION display_state_handler(TX_CONSTHANDLE async_data_handle, TX_USERPARAM param);
    void process_display_state(TX_HANDLE display_state_handle);

    using num = double;

    template<typename funs_seq, typename bounds_seq>
    static num piecewise(num x, const funs_seq& funs, const bounds_seq& bounds);

    using fun_t = std::function<num(num)>;

    num gain(num x);

    settings s;
    rel_settings rel_s;

    TX_CONTEXTHANDLE dev_ctx;
    TX_TICKET conn_state_changed_ticket;
    TX_TICKET event_handler_ticket;
    TX_HANDLE state_snapshot;
    TX_HANDLE display_state;

    QMutex global_state_mtx;

    Timer t;

    struct state
    {
        TX_REAL display_res_x, display_res_y;
        TX_REAL px, py;
        TX_REAL last_timestamp;
        bool fresh;

        state() : display_res_x(-1), display_res_y(-1), px(-1), py(-1), last_timestamp(0), fresh(false) {}
        bool is_valid() const { return !(display_res_x < 0 || px < 0); }
    } dev_state;

    double yaw, pitch;
    volatile bool do_center;
};

class tobii_eyex_dialog final : public ITrackerDialog
{
    Q_OBJECT
public:
    tobii_eyex_dialog();
    void register_tracker(ITracker *) override {}
    void unregister_tracker() override {}
private:
    Ui::tobii_eyex_dialog_widgets ui;
    settings s;
    rel_settings rs;
private slots:
    void do_ok();
    void do_cancel();
};

class tobii_eyex_metadata : public Metadata
{
public:
    QString name() { return QString("Tobii EyeX"); }
    QIcon icon() { return QIcon(":/images/tobii-eyex-logo.png"); }
};

