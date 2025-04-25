#include "export.hpp"
#include "mutex.hpp"
#include <cstdlib>
#include <QMutex>
#include <QRecursiveMutex>

template<typename M>
mutex<M>& mutex<M>::operator=(const mutex& rhs)
{
    return *this;
}

template<typename MutexType> MutexType* mutex<MutexType>::operator&() const { return &inner; }

template<typename M> mutex<M>::mutex(const mutex<M>& datum) {}
template<typename M> mutex<M>::mutex() = default;

template<typename M>
mutex<M>::operator M*() const noexcept
{
    return &inner;
}

template<typename M>
M* mutex<M>::operator->() const noexcept
{
    return &inner;
}

template class OTR_COMPAT_EXPORT mutex<QMutex>;
template class OTR_COMPAT_EXPORT mutex<QRecursiveMutex>;
