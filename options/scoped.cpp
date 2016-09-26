#include "scoped.hpp"
#include <QApplication>
#include <QThread>

// for std::abort()
#include <cstdlib>

#include <QDebug>

namespace options {

std::atomic_bool opts::teardown_flag(false);

void opts::set_teardown_flag(bool value)
{
    ensure_thread();

    // we don't use exceptions in the whole project so no need for unwind protection
    // also the calls aren't nested so no need for CAS either
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
