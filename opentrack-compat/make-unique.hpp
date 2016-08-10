#pragma once

// GNU 5.4.0 doesn't have std::make_unique in -std=c++14 mode

// this implementation was taken from http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2013/n3656.htm

#include <memory>
#include <utility>
#include <cstddef>

namespace detail {
template<class T> struct Unique_if
{
    typedef std::unique_ptr<T> Single_object;
};

template<class T> struct Unique_if<T[]>
{
    typedef std::unique_ptr<T[]> Unknown_bound;
};

template<class T, size_t N> struct Unique_if<T[N]>
{
    typedef void Known_bound;
};
}

template<class T, class... Args>
    typename detail::Unique_if<T>::Single_object
    make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }

template<class T>
    typename detail::Unique_if<T>::Unknown_bound
    make_unique(std::size_t n) {
        typedef typename std::remove_extent<T>::type U;
        return std::unique_ptr<T>(new U[n]());
    }

template<class T, class... Args>
    typename detail::Unique_if<T>::Known_bound
    make_unique(Args&&...) = delete;
