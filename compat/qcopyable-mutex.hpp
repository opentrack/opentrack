#pragma once

#include <memory>

#include <QMutex>

#include "export.hpp"

class OTR_COMPAT_EXPORT mutex
{
    std::unique_ptr<QMutex> inner;

public:
    enum mode
    {
        recursive = QMutex::Recursive,
        normal = QMutex::NonRecursive,
    };

    mutex& operator=(const mutex& datum);
    mutex(const mutex& datum);
    mutex(mode m = normal);

    QMutex* operator&() const;
    operator QMutex*() const;
    QMutex* operator->() const;
};
