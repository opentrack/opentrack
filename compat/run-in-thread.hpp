#pragma once

/* Copyright (c) 2016 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "macros.hpp"

#include <cassert>
#include <thread>
#include <condition_variable>
#include <utility>

#include <QObject>
#include <QThread>

namespace qt_impl_detail {

template<typename u>
struct run_in_thread_traits
{
    using ret_type = std::remove_reference_t<u>;

    template<typename t>
    static inline void assign(u& lvalue, t&& rvalue) { std::forward<u>(lvalue) = std::forward<t>(rvalue); }

    template<typename t>
    static inline auto pass(t&& val) -> decltype(auto) { return std::forward<t>(val); }

    template<typename F> static inline auto call(F&& fun) -> decltype(auto) { return std::forward<F>(fun)(); }
};

template<>
struct run_in_thread_traits<void>
{
    using type = unsigned char;
    using ret_type = void;
    static inline void assign(unsigned char&, unsigned char) {}
    static inline void pass(type) {}
    template<typename F> static type call(F&& fun) { std::forward<F>(fun)(); return type(0); }
};

}

template<typename F>
auto never_inline
run_in_thread_sync(QObject* obj, F&& fun)
    -> typename qt_impl_detail::run_in_thread_traits<decltype(fun())>::ret_type
{
    using lock_guard = std::unique_lock<std::mutex>;

    using traits = qt_impl_detail::run_in_thread_traits<decltype(fun())>;

    typename traits::type ret;

    struct semaphore final
    {
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

    if (obj->thread() == QThread::currentThread())
        return traits::pass(traits::call(fun));

    semaphore sem;

    {
        QObject src;
        src.moveToThread(QThread::currentThread());
        QObject::connect(&src, &QObject::destroyed, obj, [&] {
            traits::assign(ret, traits::call(fun));
            sem.notify();
        },
        Qt::QueuedConnection);
    }

    sem.wait();
    return traits::pass(std::move(ret));
}

template<typename F>
void run_in_thread_async(QObject* obj, F&& fun)
{
    if (obj->thread() == QThread::currentThread())
        return (void)fun();

    QObject src;
    QObject::connect(&src, &QObject::destroyed, obj, std::forward<F>(fun), Qt::QueuedConnection);
}
