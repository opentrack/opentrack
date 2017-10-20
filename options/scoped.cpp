#include "scoped.hpp"
#include <QApplication>
#include <QThread>

#include <cstdlib>
#include <atomic>

#include <QDebug>

namespace options {

// XXX hack: the flag shouldn't be here as action at distance -sh 20160926
static std::atomic_bool teardown_flag(false);
static void set_teardown_flag(bool value);
static void ensure_thread();
static bool is_tracker_teardown();

static void set_teardown_flag(bool value)
{
    // flag being set means "opts" is about to go out of scope due to tracker stop
    // in this case we don't reload options. we only want to reload when cancel is pressed.
    ensure_thread();
    teardown_flag = value;
}

static void ensure_thread()
{
    // only as a bug check

    if (qApp == nullptr)
        abort();

    const QThread* ui_thread = qApp->thread();
    const QThread* curthread = QThread::currentThread();

    if (ui_thread == nullptr)
        abort();

    if (ui_thread != curthread)
        abort();
}

static bool is_tracker_teardown()
{
    return teardown_flag;
}

opts::~opts()
{
    if (!is_tracker_teardown())
        b->reload();
#if 0
    else
        qDebug() << "in teardown, not reloading" << b->name();
#endif
}

opts::opts(const QString &name) : b(make_bundle(name))
{
}

with_tracker_teardown::with_tracker_teardown() : old_value(teardown_flag)
{
    set_teardown_flag(true);
}

with_tracker_teardown::~with_tracker_teardown()
{
    set_teardown_flag(old_value);
}

} // ns options
