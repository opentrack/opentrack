#pragma once

#include "export.hpp"

template<typename MutexType>
class OTR_COMPAT_EXPORT mutex
{
    mutable MutexType inner{};

public:
    mutex& operator=(const mutex& datum);
    MutexType* operator&() const;
    mutex(const mutex& datum);
    explicit mutex();

    explicit operator MutexType*() const noexcept;
    MutexType* operator->() const noexcept;
};
