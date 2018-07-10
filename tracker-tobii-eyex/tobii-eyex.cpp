#include "tobii-eyex.hpp"
#include "compat/math-imports.hpp"

#include <tuple>
#include <cstdlib>
#include <cstdio>

#include <QDebug>
#include <QMutexLocker>
#include <QMessageBox>

//#define TOBII_EYEX_DEBUG_PRINTF
#define TOBII_EYEX_VERBOSE_PRINTF

#ifdef TOBII_EYEX_VERBOSE_PRINTF
#   define dbg_verbose(msg) (qDebug() << "tobii-eyex:" << (msg))
#else
#   define dbg_verbose(msg) (void)0
#endif

#ifdef TOBII_EYEX_DEBUG2_PRINTF
#   define dbg_debug(msg) (qDebug() << "tobii-eyex:" << (msg))
#else
#   define dbg_debug(msg) (void)0
#endif

#ifdef TOBII_EYEX_DEBUG1_PRINTF
#   define dbg_debug(msg) (qDebug() << "tobii-eyex:" << (msg))
#else
#   define dbg_debug(msg) (void)0
#endif

#define dbg_notice(msg) (qDebug() << "tobii-eyex:" << (msg))

std::atomic_flag tobii_eyex_tracker::atexit_done = ATOMIC_FLAG_INIT;

static inline tobii_eyex_tracker& to_self(TX_USERPARAM param)
{
    return *reinterpret_cast<tobii_eyex_tracker*>(param);
}

tobii_eyex_tracker::tobii_eyex_tracker() = default;

void tobii_eyex_tracker::call_tx_deinit()
{
    dbg_notice("uninitialize in atexit at _fini time");
    (void) txUninitializeEyeX();
}

tobii_eyex_tracker::~tobii_eyex_tracker()
{
    dbg_verbose("dtor");

    (void) txDisableConnection(ctx);
    (void) txReleaseObject(&state_snapshot);

    bool status = true;
    status &= txShutdownContext(ctx, TX_CLEANUPTIMEOUT_FORCEIMMEDIATE, TX_FALSE) == TX_RESULT_OK;
    status &= txReleaseContext(&ctx) == TX_RESULT_OK;

    // the API cleanup function needs to be called exactly once over image lifetime.
    // client software communicates with a service and a desktop program.
    // API is ambiguous as to what happens if the image doesn't call it or crashes.
    if (!atexit_done.test_and_set())
        std::atexit(call_tx_deinit);

    if (!status)
        dbg_notice("tobii-eyex: can't shutdown properly");
}

bool tobii_eyex_tracker::register_state_snapshot(TX_CONTEXTHANDLE dev_ctx, TX_HANDLE* state_snapshot_ptr)
{
    TX_HANDLE handle = TX_EMPTY_HANDLE;
    TX_GAZEPOINTDATAPARAMS params = { TX_GAZEPOINTDATAMODE_LIGHTLYFILTERED };

    bool status = true;

    status &= txCreateGlobalInteractorSnapshot(dev_ctx, client_id, state_snapshot_ptr, &handle) == TX_RESULT_OK;
    status &= txCreateGazePointDataBehavior(handle, &params) == TX_RESULT_OK;

    (void) txReleaseObject(&handle);

    return status;
}

void tobii_eyex_tracker::process_display_state(TX_HANDLE display_state_handle)
{
    TX_SIZE2 screen_res;

    if (txGetStateValueAsSize2(display_state_handle, TX_STATEPATH_EYETRACKINGSCREENBOUNDS, &screen_res) == TX_RESULT_OK)
    {
        dbg_verbose("got display resolution") << screen_res.Width << screen_res.Height;

        QMutexLocker l(&global_state_mtx);

        dev_state.display_res_x = screen_res.Width;
        dev_state.display_res_y = screen_res.Height;
    }
    else
        dbg_notice("can't get display resolution");
}

void tobii_eyex_tracker::display_state_handler(TX_CONSTHANDLE async_data_handle, TX_USERPARAM param)
{
    tobii_eyex_tracker& self = to_self(param);

    TX_RESULT result = TX_RESULT_UNKNOWN;
    TX_HANDLE state = TX_EMPTY_HANDLE;

    if (txGetAsyncDataResultCode(async_data_handle, &result) == TX_RESULT_OK &&
        txGetAsyncDataContent(async_data_handle, &state) == TX_RESULT_OK)
    {
        self.process_display_state(state);
        txReleaseObject(&state);
    }
    else
        dbg_notice("error in display state handler");
}

void tobii_eyex_tracker::snapshot_committed_handler(TX_CONSTHANDLE async_data_handle, TX_USERPARAM)
{
    TX_RESULT result = TX_RESULT_UNKNOWN;
    txGetAsyncDataResultCode(async_data_handle, &result);

    if (!(result == TX_RESULT_OK || result == TX_RESULT_CANCELLED))
        dbg_notice("snapshot bad result code") << result;
}

void tobii_eyex_tracker::state_change_handler(TX_CONNECTIONSTATE state, TX_USERPARAM param)
{
    tobii_eyex_tracker& self = to_self(param);

    switch (state)
    {
    case TX_CONNECTIONSTATE_CONNECTED:
    {
        bool status = txCommitSnapshotAsync(self.state_snapshot, snapshot_committed_handler, param) == TX_RESULT_OK;
        if (!status)
            dbg_notice("connected but failed to initialize data stream");
        else
        {
            txGetStateAsync(self.ctx, TX_STATEPATH_EYETRACKINGSCREENBOUNDS, display_state_handler, param);
            dbg_notice("connected, data stream ok");
        }
    }
        break;
    case TX_CONNECTIONSTATE_DISCONNECTED:
        dbg_notice("connection state is now disconnected");
        break;
    case TX_CONNECTIONSTATE_TRYINGTOCONNECT:
        dbg_verbose("trying to establish connection");
        break;
    case TX_CONNECTIONSTATE_SERVERVERSIONTOOLOW:
        dbg_notice("installed driver version too low");
        break;
    case TX_CONNECTIONSTATE_SERVERVERSIONTOOHIGH:
        dbg_notice("new driver came up, we need to update sdk");
        break;
    }
}

void tobii_eyex_tracker::gaze_data_handler(TX_HANDLE gaze_data_handle)
{
    TX_GAZEPOINTDATAEVENTPARAMS params;

    if (txGetGazePointDataEventParams(gaze_data_handle, &params) == TX_RESULT_OK)
    {
        QMutexLocker l(&global_state_mtx);

        if (params.Timestamp > dev_state.last_timestamp &&
            dev_state.display_res_x > 0 &&
            // the API allows for events outside screen bounds to e.g. detect looking at keyboard.
            // closer to the screen bounds, the values get less accurate.
            // ignore events outside the screen bounds.
            params.X >= 0 && params.X < dev_state.display_res_x &&
            params.Y >= 0 && params.Y < dev_state.display_res_y)
        {
            dev_state.last_timestamp = params.Timestamp;
            dev_state.px = params.X;
            dev_state.py = params.Y;

#ifdef TOBII_EYEX_DEBUG2_PRINTF
            char buf[256] = {0};
            (void) std::sprintf(buf, "gaze data: (%.1f, %.1f)", params.X, params.Y);
            dbg_debug(buf);
#endif

            dev_state.fresh = true;
        }
    }
    else
        dbg_notice("failed to process gaze data event packet");
}

void tobii_eyex_tracker::event_handler(TX_CONSTHANDLE async_data_handle, TX_USERPARAM param)
{
    tobii_eyex_tracker& self = to_self(param);

    TX_HANDLE event_handle = TX_EMPTY_HANDLE;
    TX_HANDLE behavior_handle = TX_EMPTY_HANDLE;

    txGetAsyncDataContent(async_data_handle, &event_handle);

    if (txGetEventBehavior(event_handle, &behavior_handle, TX_BEHAVIORTYPE_GAZEPOINTDATA) == TX_RESULT_OK)
    {
        self.gaze_data_handler(behavior_handle);
        txReleaseObject(&behavior_handle);
    }

    txReleaseObject(&event_handle);
}

module_status tobii_eyex_tracker::start_tracker(QFrame*)
{
    bool status = true;

    status &= txInitializeEyeX(TX_EYEXCOMPONENTOVERRIDEFLAG_NONE, nullptr, nullptr, nullptr, nullptr) == TX_RESULT_OK;
    status &= txCreateContext(&ctx, TX_FALSE) == TX_RESULT_OK;
    status &= register_state_snapshot(ctx, &state_snapshot);
    status &= txRegisterConnectionStateChangedHandler(ctx, &state_change_ticket, state_change_handler, (TX_USERPARAM)this) == TX_RESULT_OK;
    status &= txRegisterEventHandler(ctx, &event_cookie, event_handler, (TX_USERPARAM)this) == TX_RESULT_OK;
    status &= txEnableConnection(ctx) == TX_RESULT_OK;
#if 0
    // XXX check this
    TX_CONNECTIONSTATE state;
    status &= txGetConnectionState(ctx, &state) == TX_RESULT_OK;
    status &= state == TX_CONNECTIONSTATE_CONNECTED || state == TX_CONNECTIONSTATE_TRYINGTOCONNECT;
#endif

    if (!status)
        return error(tr("Connection can't be established. device missing?"));
    else
        return status_ok();
}

bool tobii_eyex_tracker::center()
{
    do_center = true;
    return true;
}

settings::settings() : opts("tobii-eyex") {}

state::state() = default;

#include "tobii-eyex-dialog.hpp"

OPENTRACK_DECLARE_TRACKER(tobii_eyex_tracker, tobii_eyex_dialog, tobii_eyex_metadata)
