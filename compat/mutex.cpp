#include "mutex.hpp"
#include <cstdlib>

mutex& mutex::operator=(const mutex& rhs)
{
    if (rhs->isRecursive() != inner.isRecursive())
        std::abort();

    return *this;
}

mutex::mutex(const mutex& datum) : mutex{datum.inner.isRecursive() ? Recursive : NonRecursive}
{
}

mutex::mutex(RecursionMode m) : inner{m}
{
}

QMutex* mutex::operator&() const noexcept
{
    return &inner;
}

mutex::operator QMutex*() const noexcept
{
    return &inner;
}

QMutex* mutex::operator->() const noexcept
{
    return &inner;
}
