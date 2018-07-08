#include "copyable-mutex.hpp"

mutex& mutex::operator=(const mutex& datum)
{
    inner.emplace(datum->isRecursive() ? QMutex::Recursive : QMutex::NonRecursive);
    return *this;
}

mutex::mutex(const mutex& datum)
{
    *this = datum;
}

mutex::mutex(mutex::mode m) :
    inner { std::in_place, static_cast<QMutex::RecursionMode>(int(m)) }
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
    return const_cast<QMutex*>(&inner.value());
}

