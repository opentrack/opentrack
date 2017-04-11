/* Copyright (c) 2014-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <cmath>
#include "export.hpp"
#include <compat/util.hpp>

#include <initializer_list>
#include <type_traits>
#include <utility>

namespace mat_detail {

// `zz' param to fool into SFINAE member overload

template<int i, int j, int k, int zz>
constexpr bool equals = ((void)zz, i == k && j != k);

template<int i, int j, int min>
constexpr bool maybe_swizzle =
        (i == 1 || j == 1) && (i >= min || j >= min);

template<int i1, int j1, int i2, int j2>
constexpr bool is_vector_pair =
        (i1 == i2 && j1 == 1 && j2 == 1) || (j1 == j2 && i1 == 1 && i2 == 1);

template<int i, int j>
constexpr unsigned vector_len = i > j ? i : j;

template<int a, int b, int c, int d>
constexpr bool dim3 =
        (a == 3 || b == 3) && (c == 3 || d == 3) &&
        (a == 1 || b == 1) && (c == 1 || d == 1);

template<int h, int w, typename... ts>
constexpr bool arglist_correct = h * w == sizeof...(ts);

template<bool x, typename t>
using sfinae = typename std::enable_if<x, t>::type;

template<typename num, int h_, int w_>
class Mat
{
    static_assert(h_ > 0 && w_ > 0, "must have positive mat dimensions");
    num data[h_][w_];

#define OTR_ASSERT_SWIZZLE static_assert(P == h_ && Q == w_, "")

    Mat(std::initializer_list<num>&& init)
    {
        auto iter = init.begin();
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                data[j][i] = *iter++;
    }

public:
    // start sfinae-R-us block

    // rebinding w_ and h_ since SFINAE requires dependent variables

    template<int P = h_, int Q = w_> sfinae<equals<Q, P, 1, 0>, num>
    OTR_FLATTEN operator()(int i) const { OTR_ASSERT_SWIZZLE; return data[i][0]; }
    template<int P = h_, int Q = w_> sfinae<equals<Q, 0, 1, 0>, num&>
    OTR_FLATTEN operator()(int i) { OTR_ASSERT_SWIZZLE; return data[i][0]; }

    template<int P = h_, int Q = w_> sfinae<equals<P, Q, 1, 1>, num>
    OTR_FLATTEN operator()(int i) const { OTR_ASSERT_SWIZZLE; return data[0][i]; }
    template<int P = h_, int Q = w_> sfinae<equals<P, Q, 1, 1>, num&>
    OTR_FLATTEN operator()(int i) { OTR_ASSERT_SWIZZLE; return data[0][i]; }

    template<int P = h_, int Q = w_> sfinae<maybe_swizzle<P, Q, 1>, num>
    OTR_FLATTEN x() const { OTR_ASSERT_SWIZZLE; return operator()(0); }
    template<int P = h_, int Q = w_> sfinae<maybe_swizzle<P, Q, 1>, num&>
    OTR_FLATTEN x() { OTR_ASSERT_SWIZZLE; return operator()(0); }

    template<int P = h_, int Q = w_> sfinae<maybe_swizzle<P, Q, 2>, num>
    OTR_FLATTEN y() const { OTR_ASSERT_SWIZZLE; return operator()(1); }
    template<int P = h_, int Q = w_> sfinae<maybe_swizzle<P, Q, 2>, num&>
    OTR_FLATTEN y() { OTR_ASSERT_SWIZZLE; return operator()(1); }

    template<int P = h_, int Q = w_> sfinae<maybe_swizzle<P, Q, 3>, num>
    OTR_FLATTEN z() const { OTR_ASSERT_SWIZZLE; return operator()(2); }
    template<int P = h_, int Q = w_> sfinae<maybe_swizzle<P, Q, 3>, num&>
    OTR_FLATTEN z() { OTR_ASSERT_SWIZZLE; return operator()(2); }

    template<int P = h_, int Q = w_> sfinae<maybe_swizzle<P, Q, 4>, num>
    OTR_FLATTEN w() const { OTR_ASSERT_SWIZZLE; return operator()(3); }
    template<int P = h_, int Q = w_> sfinae<maybe_swizzle<P, Q, 4>, num&>
    OTR_FLATTEN w() { OTR_ASSERT_SWIZZLE; return operator()(3); }

    // end sfinae-R-us block

    // parameters w_ and h_ are rebound so that SFINAE occurs
    // removing them causes a compile-time error -sh 20150811

    template<int R, int S, int P = h_, int Q = w_>
    sfinae<is_vector_pair<R, S, P, Q>, num>
    dot(const Mat<num, R, S>& p2) const
    {
        OTR_ASSERT_SWIZZLE;

        num ret = 0;
        static constexpr unsigned len = vector_len<R, S>;
        for (unsigned i = 0; i < len; i++)
            ret += operator()(i) * p2(i);
        return ret;
    }

    template<int R, int S, int P = h_, int Q = w_> sfinae<dim3<P, Q, R, S>, Mat<num, 3, 1>>
    cross(const Mat<num, R, S>& p2) const
    {
        OTR_ASSERT_SWIZZLE;
        decltype(*this)& OTR_RESTRICT p1 = *this;

        return Mat<num, R, S>(p1.y() * p2.z() - p2.y() * p1.z(),
                              p2.x() * p1.z() - p1.x() * p2.z(),
                              p1.x() * p2.y() - p1.y() * p2.x());
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

    inline num operator()(unsigned j, unsigned i) const { return data[j][i]; }
    inline num& operator()(unsigned j, unsigned i) { return data[j][i]; }

    template<typename... ts, int P = h_, int Q = w_,
             typename = sfinae<arglist_correct<P, Q, ts...>, void>>
    Mat(const ts... xs)
    {
        OTR_ASSERT_SWIZZLE;
        static_assert(arglist_correct<P, Q, ts...>, "");

        std::initializer_list<num> init = { static_cast<num>(xs)... };

        *this = Mat(std::move(init));
    }

    Mat()
    {
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                data[j][i] = num(0);
    }

    Mat(const num* OTR_RESTRICT mem)
    {
        for (int j = 0; j < h_; j++)
            for (int i = 0; i < w_; i++)
                data[j][i] = mem[i*h_+j];
    }

    OTR_ALWAYS_INLINE operator num*() { return reinterpret_cast<num*>(data); }
    OTR_ALWAYS_INLINE operator const num*() const { return reinterpret_cast<const num*>(data); }

    // XXX add more operators as needed, third-party dependencies mostly
    // not needed merely for matrix algebra -sh 20141030

    template<int P = h_>
    static typename std::enable_if<P == w_, Mat<num, P, P>>::type eye()
    {
        static_assert(P == h_, "");

        Mat<num, P, P> ret;

        for (int j = 0; j < P; j++)
            for (int i = 0; i < w_; i++)
                ret.data[j][i] = 0;

        for (int i = 0; i < P; i++)
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

template<typename num>
class Quat : Mat<num, 4, 1>
{
    using quat = Quat<num>;

    enum idx { qw, qx, qy, qz };

    static quat _from_array(const num* data)
    {
        return quat(data[qw], data[qx], data[qy], data[qz]);
    }

    inline num elt(idx k) const { return operator()(k); }
    inline num& elt(idx k) { return Mat<num, 4, 1>::operator()(int(k)); }
public:
    Quat(num w, num x, num y, num z) : Mat<num, 4, 1>(w, x, y, z)
    {
    }

    Quat() : quat(1, 0, 0, 0) {}

    static quat from_vector(const Mat<num, 4, 1>& data)
    {
        return _from_array(data);
    }

    static quat from_vector(const Mat<num, 1, 4>& data)
    {
        return _from_array(data);
    }

    quat normalized() const
    {
        const num x = elt(qx), y = elt(qy), z = elt(qz), w = elt(qw);
        const num inv_n = 1./std::sqrt(x*x + y*y + z*z + w*w);

        return Quat<num>(elt(qw) * inv_n,
                         elt(qx) * inv_n,
                         elt(qy) * inv_n,
                         elt(qz) * inv_n);
    }

    quat operator*(const quat& q2)
    {
        const quat& OTR_RESTRICT q1 = *this;
        return quat(-q1.x() * q2.x() - q1.y() * q2.y() - q1.z() * q2.z() + q1.w() * q2.w(),
                    q1.x() * q2.w() + q1.y() * q2.z() - q1.z() * q2.y() + q1.w() * q2.x(),
                    -q1.x() * q2.z() + q1.y() * q2.w() + q1.z() * q2.x() + q1.w() * q2.y(),
                    q1.x() * q2.y() - q1.y() * q2.x() + q1.z() * q2.w() + q1.w() * q2.z());
    }

    inline num w() const { return elt(qw); }
    inline num x() const { return elt(qx); }
    inline num y() const { return elt(qy); }
    inline num z() const { return elt(qz); }

    inline num& w() { return elt(qw); }
    inline num& x() { return elt(qx); }
    inline num& y() { return elt(qy); }
    inline num& z() { return elt(qz); }
};

} // ns detail

template<typename num, int h, int w>
using Mat = mat_detail::Mat<num, h, w>;

template<typename num, int h, int w>
inline Mat<num, h, w> operator*(num scalar, const Mat<num, h, w>& mat) { return mat * scalar; }

template<typename num, int h, int w>
inline Mat<num, h, w> operator-(num scalar, const Mat<num, h, w>& mat) { return mat - scalar; }

template<typename num, int h, int w>
inline Mat<num, h, w> operator+(num scalar, const Mat<num, h, w>& mat) { return mat + scalar; }

template<typename num, int h_, int w_>
inline Mat<num, h_, w_> operator*(const Mat<num, h_, w_>& OTR_RESTRICT self, num other)
{
    Mat<num, h_, w_> ret;
    for (int j = 0; j < h_; j++)
        for (int i = 0; i < w_; i++)
            ret(j, i) = self(j, i) * other;
    return ret;
}

template<typename num, int h_, int w_>
inline Mat<num, h_, w_> operator-(const Mat<num, h_, w_>& OTR_RESTRICT self, num other)
{
    Mat<num, h_, w_> ret;
    for (int j = 0; j < h_; j++)
        for (int i = 0; i < w_; i++)
            ret(j, i) = self(j, i) - other;
    return ret;
}

template<typename num, int h_, int w_>
inline Mat<num, h_, w_> operator+(const Mat<num, h_, w_>& OTR_RESTRICT self, num other)
{
    Mat<num, h_, w_> ret;
    for (int j = 0; j < h_; j++)
        for (int i = 0; i < w_; i++)
            ret(j, i) = self(j, i) + other;
    return ret;
}

template<typename num>
using Quat_ = mat_detail::Quat<num>;

using Quat = Quat_<double>;

template class mat_detail::Mat<float, 3, 3>;
template class mat_detail::Mat<float, 6, 1>;
template class mat_detail::Mat<float, 3, 1>;

// eof
