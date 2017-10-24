#include "copyable-mutex.hpp"

mutex& mutex::operator=(const mutex& datum)
{
    inner = nullptr;
    inner = std::make_unique<QMutex>(datum->isRecursive()
                                     ? QMutex::Recursive
                                     : QMutex::NonRecursive);
    return *this;
}

mutex::mutex(const mutex& datum)
{
    *this = datum;
}

mutex::mutex(mutex::mode m) :
    inner(std::make_unique<QMutex>(static_cast<QMutex::RecursionMode>(int(m))))
{
}

QMutex* mutex::operator&() const
{
    return *this;
}

QMutex* mutex::operator->() const
{
    return *this;
}

mutex::operator QMutex*() const
{
    return const_cast<QMutex*>(inner.get());
}

