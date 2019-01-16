#pragma once

#include <QMutex>

#include "export.hpp"

class OTR_COMPAT_EXPORT mutex
{
    mutable QMutex inner;

public:
    using RecursionMode = QMutex::RecursionMode;
    static constexpr RecursionMode Recursive = RecursionMode::Recursive;
    static constexpr RecursionMode NonRecursive = RecursionMode::NonRecursive;

    mutex& operator=(const mutex& datum);
    mutex(const mutex& datum);
    explicit mutex(RecursionMode m);
    mutex() : mutex{NonRecursive} {}

    QMutex* operator&() const noexcept;
    explicit operator QMutex*() const noexcept;
    QMutex* operator->() const noexcept;
};
