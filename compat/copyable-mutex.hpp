#pragma once

#include <optional>

#include <QMutex>

#include "export.hpp"

class OTR_COMPAT_EXPORT mutex
{
    std::optional<QMutex> inner;

public:
    enum mode
    {
        recursive = QMutex::Recursive,
        normal = QMutex::NonRecursive,
    };

    mutex& operator=(const mutex& datum);
    mutex(const mutex& datum);
    explicit mutex(mode m = normal);

    QMutex* operator&() const;
    operator QMutex*() const;
    QMutex* operator->() const;
};
