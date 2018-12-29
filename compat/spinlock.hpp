#pragma once

#include "macros1.h"
#include <atomic>

struct spinlock_guard final
{
    spinlock_guard(const spinlock_guard&) = delete;
    spinlock_guard& operator=(const spinlock_guard&) = delete;
    constexpr spinlock_guard(spinlock_guard&&) noexcept = default;

    cc_forceinline
    spinlock_guard(std::atomic_flag* lock) noexcept : spinlock_guard(*lock) {}

    cc_forceinline
    spinlock_guard(std::atomic_flag& lock) noexcept : lock(lock)
    {
        while (lock.test_and_set(std::memory_order_acquire))
            (void)0;
    }

    cc_forceinline
    ~spinlock_guard() noexcept
    {
        lock.clear(std::memory_order_release);
    }

private:
    std::atomic_flag& lock;
};
