#include "scoped.hpp"
#include <QApplication>
#include <QThread>

#include <cstdlib>
#include <atomic>

#include <QDebug>

namespace options {

std::atomic_bool opts::teardown_flag(false);

void opts::set_teardown_flag(bool value)
{
    // flag being set means "opts" is about to go out of scope due to tracker stop
    // in this case we don't reload options. we only want to reload when cancel is pressed.
    ensure_thread();
    teardown_flag = value;
}

void opts::ensure_thread()
{
    // only as a bug check

    const QThread* ui_thread = qApp->thread();
    const QThread* curthread = QThread::currentThread();

    if (ui_thread == nullptr)
        abort();

    if (ui_thread != curthread)
        abort();
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

bool opts::is_tracker_teardown()
{
    return teardown_flag;
}

opts::opts(const QString &name) : b(make_bundle(name))
{
}

}
