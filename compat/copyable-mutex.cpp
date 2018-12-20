#include "copyable-mutex.hpp"
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

QMutex* mutex::operator&() const
{
    return *this;
}

mutex::operator QMutex*() const
{
    return &inner;
}

QMutex* mutex::operator->() const
{
    return *this;
}
