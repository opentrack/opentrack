#pragma once

#include <QObject>

#include <thread>
#include <condition_variable>
#include <utility>

namespace detail {

template<typename t>
struct run_in_thread_traits
{
    using type = t;
    using ret_type = t&&;
    static inline void assign(t& lvalue, t&& rvalue) { lvalue = rvalue; }
    static inline ret_type&& pass(ret_type&& val) { return std::move(val); }
    template<typename F> static ret_type call(F& fun) { return std::move(fun()); }
};

template<>
struct run_in_thread_traits<void>
{
    using type = unsigned char;
    using ret_type = void;
    static inline void assign(unsigned char&, unsigned char&&) {}
    static inline void pass(type&&) {}
    template<typename F> static type&& call(F& fun) { fun(); return std::move(type(0)); }
};

}

template<typename F>
auto run_in_thread_sync(QObject* obj, F&& fun)
    -> typename detail::run_in_thread_traits<decltype(std::forward<F>(fun)())>::ret_type
{
    using lock_guard = std::unique_lock<std::mutex>;

    std::mutex mtx;
    lock_guard guard(mtx);
    std::condition_variable cvar;

    std::thread::id waiting_thread = std::this_thread::get_id();

    using traits = detail::run_in_thread_traits<decltype(std::forward<F>(fun)())>;

    typename traits::type ret;

    bool skip_wait = false;

    {
        QObject src;
        src.moveToThread(obj->thread());
        QObject::connect(&src,
                         &QObject::destroyed,
                         obj,
                         [&]() {
            std::thread::id calling_thread = std::this_thread::get_id();
            if (waiting_thread == calling_thread)
            {
                skip_wait = true;
                traits::assign(ret, traits::call(fun));
            }
            else
            {
                lock_guard guard(mtx);
                traits::assign(ret, traits::call(fun));
                cvar.notify_one();
            }
        },
        Qt::AutoConnection);
    }

    if (!skip_wait)
        cvar.wait(guard);
    return traits::pass(std::move(ret));
}

template<typename F>
void run_in_thread_async(QObject* obj, F&& fun)
{
    QObject src;
    src.moveToThread(obj->thread());
    QObject::connect(&src, &QObject::destroyed, obj, std::move(fun), Qt::AutoConnection);
}
