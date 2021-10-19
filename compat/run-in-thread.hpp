#pragma once

/* Copyright (c) 2016 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include <thread>
#include <condition_variable>
#include <utility>

#include <QObject>
#include <QThread>

namespace impl_run_in_thread {
struct semaphore final
{
    using lock_guard = std::unique_lock<std::mutex>;
    std::mutex mtx;
    std::condition_variable cvar;
    bool flag = false;

    semaphore() = default;

    void wait()
    {
        lock_guard guard(mtx);
        while (!flag)
            cvar.wait(guard);
    }

    void notify()
    {
        lock_guard guard(mtx);
        flag = true;
        cvar.notify_one();
    }
};
}

template<typename F>
void run_in_thread_sync(QObject* obj, F&& fun)
{
    if (obj->thread() == QThread::currentThread())
        return (void)fun();

    impl_run_in_thread::semaphore sem;

    {
        QObject src;
        QObject::connect(&src, &QObject::destroyed,
                         obj, [&] { fun(); sem.notify(); },
                         Qt::QueuedConnection);
    }

    sem.wait();
}

template<typename F>
void run_in_thread_async(QObject* obj, F&& fun)
{
    if (obj->thread() == QThread::currentThread())
        return (void)fun();

    QObject src;
    QObject::connect(&src, &QObject::destroyed, obj, std::forward<F>(fun), Qt::QueuedConnection);
}
