/* Copyright (c) 2014-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "export.hpp"

#include <type_traits>
#include <utility>
#include <cmath>

namespace simple_mat_detail {
    // last param to fool SFINAE into overloading
    template<int i, int j, int>
    struct equals
    {
        enum { value = i == j };
    };
    template<int i, int j, int min>
    struct maybe_add_swizzle
    {
        enum { value = (i == 1 || j == 1) && (i >= min || j >= min) };
    };
    template<int i1, int j1, int i2, int j2>
    struct is_vector_pair
    {
        enum { value = (i1 == i2 && j1 == 1 && j2 == 1) || (j1 == j2 && i1 == 1 && i2 == 1) };
    };
    template<int i, int j>
    struct vector_len
    {
        enum { value = i > j ? i : j };
    };
    template<int a, int b, int c, int d>
    struct is_dim3
    {
        enum { value = (a == 1 && c == 1 && b == 3 && d == 3) || (a == 3 && c == 3 && b == 1 && d == 1) };
        enum { P = a == 1 ? 1 : 3 };
        enum { Q = a == 1 ? 3 : 1 };
    };

    template<typename num, int h, int w, typename...ts>
    struct is_arglist_correct
    {
        enum { value = h * w == sizeof...(ts) };
    };

template<typename num, int h_, int w_>
class Mat
{
    static_assert(h_ > 0 && w_ > 0, "must have positive mat dimensions");
    num data[h_][w_];

public:
    template<int Q = w_> std::enable_if_t<equals<Q, 1, 0>::value, num>
    constexpr inline operator()(int i) const& { return data[i][0]; }

    template<int P = h_> std::enable_if_t<equals<P, 1, 1>::value, num>
    constexpr inline operator()(int i) const& { return data[0][i]; }

    template<int Q = w_> std::enable_if_t<equals<Q, 1, 2>::value, num&>
    constexpr inline operator()(int i) & { return data[i][0]; }

    template<int P = h_> std::enable_if_t<equals<P, 1, 3>::value, num&>
    constexpr inline operator()(int i) & { return data[0][i]; }

    template<int Q = w_> std::enable_if_t<equals<Q, 1, 0>::value, num>
    constexpr inline operator()(unsigned i) const& { return data[i][0]; }

    template<int P = h_> std::enable_if_t<equals<P, 1, 1>::value, num>
    constexpr inline operator()(unsigned i) const& { return data[0][i]; }

    template<int Q = w_> std::enable_if_t<equals<Q, 1, 2>::value, num&>
    constexpr inline operator()(unsigned i) & { return data[i][0]; }

    template<int P = h_> std::enable_if_t<equals<P, 1, 3>::value, num&>
    constexpr inline operator()(unsigned i) & { return data[0][i]; }

#define OPENTRACK_ASSERT_SWIZZLE static_assert(P == h_ && Q == w_, "")

    template<int P = h_, int Q = w_> std::enable_if_t<maybe_add_swizzle<P, Q, 1>::value, num>
    constexpr inline x() const& { OPENTRACK_ASSERT_SWIZZLE; return operator()(0); }

    template<int P = h_, int Q = w_> std::enable_if_t<maybe_add_swizzle<P, Q, 2>::value, num>
    constexpr inline y() const& { OPENTRACK_ASSERT_SWIZZLE; return operator()(1); }

    template<int P = h_, int Q = w_> std::enable_if_t<maybe_add_swizzle<P, Q, 3>::value, num>
    constexpr inline z() const& { OPENTRACK_ASSERT_SWIZZLE; return operator()(2); }

    template<int P = h_, int Q = w_> std::enable_if_t<maybe_add_swizzle<P, Q, 4>::value, num>
    constexpr inline w() const& { OPENTRACK_ASSERT_SWIZZLE; return operator()(3); }

    template<int P = h_, int Q = w_> std::enable_if_t<maybe_add_swizzle<P, Q, 1>::value, num&>
    constexpr inline x() & { OPENTRACK_ASSERT_SWIZZLE; return operator()(0); }

    template<int P = h_, int Q = w_> std::enable_if_t<maybe_add_swizzle<P, Q, 2>::value, num&>
    constexpr inline y() & { OPENTRACK_ASSERT_SWIZZLE; return operator()(1); }

    template<int P = h_, int Q = w_> std::enable_if_t<maybe_add_swizzle<P, Q, 3>::value, num&>
    constexpr inline z() & { OPENTRACK_ASSERT_SWIZZLE; return operator()(2); }

    template<int P = h_, int Q = w_> std::enable_if_t<maybe_add_swizzle<P, Q, 4>::value, num&>
    constexpr inline w() & { OPENTRACK_ASSERT_SWIZZLE; return operator()(3); }

    // parameters w_ and h_ are rebound so that SFINAE occurs
    // removing them causes a compile-time error -sh 20150811

    template<int R, int S, int P = h_, int Q = w_>
    std::enable_if_t<is_vector_pair<R, S, P, Q>::value, num>
    norm() const
    {
        static_assert(P == h_ && Q == w_, "");

        const num val = dot(*this);

        if (val < num(1e-4))
            return num(0);
        else
            return std::sqrt(val);
    }

    template<int R, int S, int P = h_, int Q = w_>
    std::enable_if_t<is_vector_pair<R, S, P, Q>::value, num>
    constexpr dot(const Mat<num, R, S>& p2) const
    {
        static_assert(P == h_ && Q == w_, "");

        num ret = 0;
        constexpr int len = vector_len<R, S>::value;
        for (int i = 0; i < len; i++)
            ret += operator()(i) * p2(i);
        return ret;
    }

    template<int R, int S, int P = h_, int Q = w_>
    std::enable_if_t<is_dim3<P, Q, R, S>::value, Mat<num, is_dim3<P, Q, R, S>::P, is_dim3<P, Q, R, S>::Q>>
    constexpr cross(const Mat<num, R, S>& b) const
    {
        static_assert(P == h_ && Q == w_, "");
        auto& a = *this;

        return Mat<num, R, S>(a.y()*b.z() - a.z()*b.y(),
                              a.z()*b.x() - a.x()*b.z(),
                              a.x()*b.y() - a.y()*b.x());
    }

    constexpr Mat<num, h_, w_> operator+(const Mat<num, h_, w_>& other) const
    {
        Mat<num, h_, w_> ret;
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                ret(j, i) = data[j][i] + other.data[j][i];
        return ret;
    }

    constexpr Mat<num, h_, w_> operator-(const Mat<num, h_, w_>& other) const
    {
        Mat<num, h_, w_> ret;
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                ret(j, i) = data[j][i] - other.data[j][i];
        return ret;
    }

    constexpr Mat<num, h_, w_> operator+(const num& other) const
    {
        Mat<num, h_, w_> ret;
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                ret(j, i) = data[j][i] + other;
        return ret;
    }

    constexpr Mat<num, h_, w_> operator-(const num& other) const
    {
        Mat<num, h_, w_> ret;
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                ret(j, i) = data[j][i] - other;
        return ret;
    }

    template<int p>
    constexpr Mat<num, h_, p> operator*(const Mat<num, w_, p>& other) const
    {
        Mat<num, h_, p> ret;
        for (int k = 0; k < h_; k++)
            for (int i = 0; i < p; i++)
            {
                ret(k, i) = 0;
                for (int j = 0; j < w_; j++)
                    ret(k, i) += data[k][j] * other(j, i);
            }
        return ret;
    }

    constexpr Mat<num, h_, w_> mult_elementwise(const Mat<num, h_, w_>& other) const&
    {
        Mat<num, h_, w_> ret;

        for (unsigned j = 0; j < h_; j++)
            for (unsigned i = 0; i < w_; i++)
                ret(j, i) = data[j][i] * other.data[j][i];

        return ret;
    }

    constexpr inline num operator()(int j, int i) const& { return data[j][i]; }
    constexpr inline num& operator()(int j, int i) & { return data[j][i]; }

    constexpr inline num operator()(unsigned j, unsigned i) const& { return data[j][i]; }
    constexpr inline num& operator()(unsigned j, unsigned i) & { return data[j][i]; }

#ifdef __GNUG__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wmissing-braces"
#endif

    template<typename... ts, int h__ = h_, int w__ = w_,
             typename = std::enable_if_t<is_arglist_correct<num, h__, w__, ts...>::value>>
    constexpr Mat(const ts... xs) : data{static_cast<num>(xs)...}
    {
        static_assert(h__ == h_ && w__ == w_, "");
    }

#ifdef __GNUG__
#   pragma GCC diagnostic pop
#endif

    constexpr Mat()
    {
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                data[j][i] = num(0);
    }

    Mat(const num* mem)
    {
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                data[j][i] = mem[i*h_+j];
    }

    operator num*() { return reinterpret_cast<num*>(data); }
    operator const num*() const { return reinterpret_cast<const num*>(data); }

    // XXX add more operators as needed, third-party dependencies mostly
    // not needed merely for matrix algebra -sh 20141030

    template<int h__ = h_>
    static std::enable_if_t<h_ == w_, Mat<num, h__, h__>> eye()
    {
        static_assert(h_ == h__, "");

        Mat<num, h_, h_> ret;
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                ret.data[j][i] = 0;

        for (int i = 0; i < h_; i++)
            ret.data[i][i] = 1;

        return ret;
    }

    constexpr Mat<num, w_, h_> t() const
    {
        Mat<num, w_, h_> ret;

        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                ret(i, j) = data[j][i];

        return ret;
    }
};

template<typename num, int h, int w>
constexpr Mat<num, h, w> operator*(num scalar, const Mat<num, h, w>& mat)
{
    return mat * scalar;
}

template<typename num, int h_, int w_>
constexpr Mat<num, h_, w_> operator*(const Mat<num, h_, w_>& self, num other)
{
    Mat<num, h_, w_> ret;
    for (int j = 0; j < h_; j++)
        for (int i = 0; i < w_; i++)
            ret(j, i) = self(j, i) * other;
    return ret;
}

} // ns simple_mat_detail

template<typename num, int h, int w>
using Mat = simple_mat_detail::Mat<num, h, w>;
