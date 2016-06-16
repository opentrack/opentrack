/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once
#include <initializer_list>
#include <type_traits>
#include <cmath>

namespace {
    // last param to fool SFINAE into overloading
    template<int i, int j, int ignored>
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
}

template<typename num, int h_, int w_>
class Mat
{
    num data[h_][w_];

    static_assert(h_ > 0 && w_ > 0, "must have positive mat dimensions");

    Mat(std::initializer_list<num>&& xs) = delete;

public:

    // parameters w_ and h_ are rebound so that SFINAE occurs
    // removing them causes a compile-time error -sh 20150811

    template<int Q = w_> typename std::enable_if<equals<Q, 1, 0>::value, num>::type
    inline operator()(int i) const { return data[i][0]; }

    template<int P = h_> typename std::enable_if<equals<P, 1, 1>::value, num>::type
    inline operator()(int i) const { return data[0][i]; }

    template<int Q = w_> typename std::enable_if<equals<Q, 1, 2>::value, num&>::type
    inline operator()(int i) { return data[i][0]; }

    template<int P = h_> typename std::enable_if<equals<P, 1, 3>::value, num&>::type
    inline operator()(int i) { return data[0][i]; }

    template<int P = h_, int Q = w_> typename std::enable_if<maybe_add_swizzle<P, Q, 1>::value, num>::type
    inline x() const { return operator()(0); }

    template<int P = h_, int Q = w_> typename std::enable_if<maybe_add_swizzle<P, Q, 2>::value, num>::type
    inline y() const { return operator()(1); }

    template<int P = h_, int Q = w_> typename std::enable_if<maybe_add_swizzle<P, Q, 3>::value, num>::type
    inline z() const { return operator()(2); }

    template<int P = h_, int Q = w_> typename std::enable_if<maybe_add_swizzle<P, Q, 4>::value, num>::type
    inline w() const { return operator()(3); }

    template<int P = h_, int Q = w_> typename std::enable_if<maybe_add_swizzle<P, Q, 1>::value, num&>::type
    inline x() { return operator()(0); }

    template<int P = h_, int Q = w_> typename std::enable_if<maybe_add_swizzle<P, Q, 2>::value, num&>::type
    inline y() { return operator()(1); }

    template<int P = h_, int Q = w_> typename std::enable_if<maybe_add_swizzle<P, Q, 3>::value, num&>::type
    inline z() { return operator()(2); }

    template<int P = h_, int Q = w_> typename std::enable_if<maybe_add_swizzle<P, Q, 4>::value, num&>::type
    inline w() { return operator()(3); }

    template<int R, int S, int P = h_, int Q = w_>
    typename std::enable_if<is_vector_pair<R, S, P, Q>::value, num>::type
    dot(const Mat<num, R, S>& p2) const {
        num ret = 0;
        constexpr int len = vector_len<R, S>::value;
        for (int i = 0; i < len; i++)
            ret += operator()(i) * p2(i);
        return ret;
    }

    template<int R, int S, int P = h_, int Q = w_>
    typename std::enable_if<is_dim3<P, Q, R, S>::value, Mat<num, is_dim3<P, Q, R, S>::P, is_dim3<P, Q, R, S>::Q>>::type
    cross(const Mat<num, R, S>& p2) const
    {
        return Mat<num, R, S>(y() * p2.z() - p2.y() * z(),
                              p2.x() * z() - x() * p2.z(),
                              x() * p2.y() - y() * p2.x());
    }

    Mat<num, h_, w_> operator+(const Mat<num, h_, w_>& other) const
    {
        Mat<num, h_, w_> ret;
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                ret(j, i) = data[j][i] + other.data[j][i];
        return ret;
    }

    Mat<num, h_, w_> operator-(const Mat<num, h_, w_>& other) const
    {
        Mat<num, h_, w_> ret;
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                ret(j, i) = data[j][i] - other.data[j][i];
        return ret;
    }

    Mat<num, h_, w_> operator+(const num& other) const
    {
        Mat<num, h_, w_> ret;
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                ret(j, i) = data[j][i] + other;
        return ret;
    }

    Mat<num, h_, w_> operator-(const num& other) const
    {
        Mat<num, h_, w_> ret;
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                ret(j, i) = data[j][i] - other;
        return ret;
    }

    Mat<num, h_, w_> operator*(const num other) const
    {
        Mat<num, h_, w_> ret;
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                ret(j, i) = data[j][i] * other;
        return ret;
    }

    template<int p>
    Mat<num, h_, p> operator*(const Mat<num, w_, p>& other) const
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

    inline num operator()(int j, int i) const { return data[j][i]; }
    inline num& operator()(int j, int i) { return data[j][i]; }

    template<typename... ts, int h__ = h_, int w__ = w_,
             typename = typename std::enable_if<is_arglist_correct<num, h__, w__, ts...>::value>::type>
    Mat(const ts... xs)
    {
        const std::initializer_list<num> init = { static_cast<num>(xs)... };
        auto iter = init.begin();
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                data[j][i] = *iter++;
    }

    Mat()
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

    Mat(num* mem) : Mat(const_cast<const num*>(mem)) {}

    template<int h = h_,
             int w = w_,
             typename = typename std::enable_if<h == 1 || w == 1>::type>
    operator const double*() const
    {
        return reinterpret_cast<const double*>(data);
    }

    // XXX add more operators as needed, third-party dependencies mostly
    // not needed merely for matrix algebra -sh 20141030

    static Mat<num, h_, h_> eye()
    {
        Mat<num, h_, h_> ret;
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                ret.data[j][i] = 0;

        for (int i = 0; i < h_; i++)
            ret.data[i][i] = 1;

        return ret;
    }

    Mat<num, w_, h_> t() const
    {
        Mat<num, w_, h_> ret;

        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                ret(i, j) = data[j][i];

        return ret;
    }
};

#include "export.hpp"

namespace euler {

template<int y, int x> using dmat = Mat<double, y, x>;
using rmat = dmat<3, 3>;
using euler_t = dmat<3, 1>;

rmat OPENTRACK_API_EXPORT euler_to_rmat(const double* input);

// http://stackoverflow.com/a/18436193
euler_t OPENTRACK_API_EXPORT rmat_to_euler(const dmat<3, 3>& R);

} // end ns euler

template<int h_, int w_> using dmat = Mat<double, h_, w_>;
