#pragma once

#include <QMutex>

#include "export.hpp"

class OTR_COMPAT_EXPORT mutex
{
    mutable QMutex inner;

public:
    using RecursionMode = QMutex::RecursionMode;
    static constexpr inline RecursionMode Recursive = RecursionMode::Recursive;
    static constexpr inline RecursionMode NonRecursive = RecursionMode::NonRecursive;

    mutex& operator=(const mutex& datum);
    mutex(const mutex& datum);
    explicit mutex(RecursionMode m);

    QMutex* operator&() const;
    operator QMutex*() const;
    QMutex* operator->() const;
};
