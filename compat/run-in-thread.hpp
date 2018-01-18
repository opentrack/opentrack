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

template<typename t>
struct run_in_thread_traits
{
    using type = t;
    using ret_type = t;
    static inline void assign(t& lvalue, const t& rvalue) { lvalue = rvalue; }
    static inline t pass(const t& val) { return val; }
    template<typename F> static inline t call(F&& fun) { return std::move(fun()); }
};

template<typename u>
struct run_in_thread_traits<u&&>
{
    using t = typename std::remove_reference<u>::type;
    using type = t;
    using ret_type = u;
    static inline void assign(t& lvalue, t&& rvalue) { lvalue = rvalue; }
    static inline t&& pass(t&& val) { return val; }
    template<typename F> static inline t&& call(F&& fun) { return std::move(fun()); }
};

template<>
struct run_in_thread_traits<void>
{
    using type = unsigned char;
    using ret_type = void;
    static inline void assign(unsigned char&, unsigned char&&) {}
    static inline void pass(type&&) {}
    template<typename F> static type call(F&& fun) { fun(); return type(0); }
};

}

template<typename F>
auto never_inline
run_in_thread_sync(QObject* obj, F&& fun)
    -> typename qt_impl_detail::run_in_thread_traits<decltype(std::forward<F>(fun)())>::ret_type
{
    using lock_guard = std::unique_lock<std::mutex>;

    using traits = qt_impl_detail::run_in_thread_traits<decltype(std::forward<F>(fun)())>;

    typename traits::type ret;

    struct semaphore final
    {
        std::mutex mtx;
        std::condition_variable cvar;
        bool flag;

        semaphore() : flag(false) {}

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

    semaphore sem;

    {
        QObject src;
        QObject::connect(&src,
                         &QObject::destroyed,
                         obj,
                         [&]() {
            traits::assign(ret, traits::call(fun));
            sem.notify();
        },
        Qt::AutoConnection);
    }

    sem.wait();
    return traits::pass(std::move(ret));
}

template<typename F>
void run_in_thread_async(QObject* obj, F&& fun)
{
    QObject src;
    QThread* t(obj->thread());
    assert(t);
    src.moveToThread(t);
    QObject::connect(&src, &QObject::destroyed, obj, std::move(fun), Qt::AutoConnection);
}
