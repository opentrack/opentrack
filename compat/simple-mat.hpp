/* Copyright (c) 2014-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>
#include <cmath>

namespace simple_mat {
    // last param to fool SFINAE into overloading
    template<int i, int j, int>
    struct equals
    {
        enum { value = i == j };
    };
    template<int i, int j, int min, int max>
    struct maybe_add_swizzle
    {
        enum { value = (i == 1 || j == 1) && (i >= min || j >= min) && (i <= max || j <= max) };
    };

    template<int j, int i>
    struct is_vector
    {
        enum { value = j == 1 || i == 1 };
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

    template<typename, int h, int w, typename...ts>
    struct is_arglist_correct
    {
        enum { value = h * w == sizeof...(ts) };
    };

template<typename num, int H, int W>
class Mat
{
    static_assert(H > 0 && W > 0, "must have positive mat dimensions");
    num data[H][W];

public:
    // parameters W and H are rebound so that SFINAE occurs
    // removing them causes a compile-time error -sh 20150811

    template<typename t, int Q = W> std::enable_if_t<equals<Q, 1, 0>::value, num>
    constexpr inline operator()(t i) const& { return data[(unsigned)i][0]; }

    template<typename t, int P = H> std::enable_if_t<equals<P, 1, 1>::value, num>
    constexpr inline operator()(t i) const& { return data[0][(unsigned)i]; }

    template<typename t, int Q = W> std::enable_if_t<equals<Q, 1, 2>::value, num&>
    constexpr inline operator()(t i) & { return data[(unsigned)i][0]; }

    template<typename t, int P = H> std::enable_if_t<equals<P, 1, 3>::value, num&>
    constexpr inline operator()(t i) & { return data[0][(unsigned)i]; }

#define OTR_MAT_ASSERT_SWIZZLE static_assert(P == H && Q == W)

    // const variants
    template<int P = H, int Q = W> std::enable_if_t<maybe_add_swizzle<P, Q, 1, 4>::value, num>
    constexpr inline x() const& { OTR_MAT_ASSERT_SWIZZLE; return operator()(0); }

    template<int P = H, int Q = W> std::enable_if_t<maybe_add_swizzle<P, Q, 2, 4>::value, num>
    constexpr inline y() const& { OTR_MAT_ASSERT_SWIZZLE; return operator()(1); }

    template<int P = H, int Q = W> std::enable_if_t<maybe_add_swizzle<P, Q, 3, 4>::value, num>
    constexpr inline z() const& { OTR_MAT_ASSERT_SWIZZLE; return operator()(2); }

    template<int P = H, int Q = W> std::enable_if_t<maybe_add_swizzle<P, Q, 4, 4>::value, num>
    constexpr inline w() const& { OTR_MAT_ASSERT_SWIZZLE; return operator()(3); }

    // mutable variants
    template<int P = H, int Q = W> std::enable_if_t<maybe_add_swizzle<P, Q, 1, 4>::value, num&>
    constexpr inline x() & { OTR_MAT_ASSERT_SWIZZLE; return operator()(0); }

    template<int P = H, int Q = W> std::enable_if_t<maybe_add_swizzle<P, Q, 2, 4>::value, num&>
    constexpr inline y() & { OTR_MAT_ASSERT_SWIZZLE; return operator()(1); }

    template<int P = H, int Q = W> std::enable_if_t<maybe_add_swizzle<P, Q, 3, 4>::value, num&>
    constexpr inline z() & { OTR_MAT_ASSERT_SWIZZLE; return operator()(2); }

    template<int P = H, int Q = W> std::enable_if_t<maybe_add_swizzle<P, Q, 4, 4>::value, num&>
    constexpr inline w() & { OTR_MAT_ASSERT_SWIZZLE; return operator()(3); }

    template<int P = H, int Q = W>
    constexpr auto norm_squared() const -> std::enable_if_t<is_vector<P, Q>::value, num>
    {
        static_assert(P == H && Q == W);

        const num val = dot(*this);
        constexpr num eps = num(1e-4);

        if (val < eps)
            return num(0);
        else
            return val;
    }

    inline auto norm() const { return num(std::sqrt(norm_squared())); }

    template<int R, int S, int P = H, int Q = W>
    std::enable_if_t<is_vector_pair<R, S, P, Q>::value, num>
    constexpr dot(const Mat<num, R, S>& p2) const
    {
        static_assert(P == H && Q == W);

        num ret = 0;
        constexpr int len = vector_len<R, S>::value;
        for (int i = 0; i < len; i++)
            ret += operator()(i) * p2(i);
        return ret;
    }

    template<int R, int S, int P = H, int Q = W>
    std::enable_if_t<is_dim3<P, Q, R, S>::value, Mat<num, is_dim3<P, Q, R, S>::P, is_dim3<P, Q, R, S>::Q>>
    constexpr cross(const Mat<num, R, S>& b) const
    {
        static_assert(P == H && Q == W);
        const auto& a = *this;

        return Mat<num, R, S>(a.y()*b.z() - a.z()*b.y(),
                              a.z()*b.x() - a.x()*b.z(),
                              a.x()*b.y() - a.y()*b.x());
    }

    constexpr Mat<num, H, W> operator+(const Mat<num, H, W>& other) const
    {
        Mat<num, H, W> ret;
        for (int j = 0; j < H; j++)
            for (int i = 0; i < W; i++)
                ret(j, i) = data[j][i] + other.data[j][i];
        return ret;
    }

    constexpr Mat<num, H, W> operator-(const Mat<num, H, W>& other) const
    {
        Mat<num, H, W> ret;
        for (int j = 0; j < H; j++)
            for (int i = 0; i < W; i++)
                ret(j, i) = data[j][i] - other.data[j][i];
        return ret;
    }

    constexpr Mat<num, H, W> operator+(const num other) const
    {
        Mat<num, H, W> ret;
        for (int j = 0; j < H; j++)
            for (int i = 0; i < W; i++)
                ret(j, i) = data[j][i] + other;
        return ret;
    }

    constexpr Mat<num, H, W> operator-(const num other) const
    {
        Mat<num, H, W> ret;
        for (int j = 0; j < H; j++)
            for (int i = 0; i < W; i++)
                ret(j, i) = data[j][i] - other;
        return ret;
    }

    template<int p>
    constexpr Mat<num, H, p> operator*(const Mat<num, W, p>& other) const
    {
        Mat<num, H, p> ret;
        for (int k = 0; k < H; k++)
            for (int i = 0; i < p; i++)
            {
                ret(k, i) = 0;
                for (int j = 0; j < W; j++)
                    ret(k, i) += data[k][j] * other(j, i);
            }
        return ret;
    }

    constexpr Mat<num, H, W> mult_elementwise(const Mat<num, H, W>& other) const&
    {
        Mat<num, H, W> ret;

        for (unsigned j = 0; j < H; j++)
            for (unsigned i = 0; i < W; i++)
                ret(j, i) = data[j][i] * other.data[j][i];

        return ret;
    }

    template<typename t, typename u>
    constexpr inline num operator()(t j, u i) const& { return data[(unsigned)j][(unsigned)i]; }

    template<typename t, typename u>
    constexpr inline num& operator()(t j, u i) & { return data[(unsigned)j][(unsigned)i]; }

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wmissing-braces"
#endif

    template<typename... ts, int h2 = H, int w2 = W,
             typename = std::enable_if_t<is_arglist_correct<num, h2, w2, ts...>::value>>
    constexpr Mat(const ts... xs) : data{static_cast<num>(xs)...}
    {
        static_assert(h2 == H && w2 == W);
    }

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

    constexpr Mat()
    {
        for (int j = 0; j < H; j++)
            for (int i = 0; i < W; i++)
                data[j][i] = num(0);
    }

    constexpr Mat(const num* mem)
    {
        for (int j = 0; j < H; j++)
            for (int i = 0; i < W; i++)
                data[j][i] = mem[i*H+j];
    }

    constexpr Mat(const Mat<num, H, W>& x)
    {
        for (int j = 0; j < H; j++)
            for (int i = 0; i < W; i++)
                data[j][i] = x(j, i);
    }

    constexpr operator num*() & { return (num*)data; }
    constexpr operator const num*() const& { return (const num*)data; }

    // XXX add more operators as needed, third-party dependencies mostly
    // not needed merely for matrix algebra -sh 20141030

    template<int H_ = H>
    static constexpr std::enable_if_t<H == W, Mat<num, H_, H_>> eye()
    {
        static_assert(H == H_);

        Mat<num, H, H> ret;
        for (int j = 0; j < H; j++)
            for (int i = 0; i < W; i++)
                ret.data[j][i] = 0;

        for (int i = 0; i < H; i++)
            ret.data[i][i] = 1;

        return ret;
    }

    constexpr Mat<num, W, H> t() const
    {
        Mat<num, W, H> ret;

        for (int j = 0; j < H; j++)
            for (int i = 0; i < W; i++)
                ret(i, j) = data[j][i];

        return ret;
    }

    constexpr Mat<num, H, W>& operator=(const Mat<num, H, W>& rhs)
    {
        for (unsigned j = 0; j < H; j++)
            for (unsigned i = 0; i < W; i++)
                data[j][i] = rhs(j, i);

        return *this;
    }
};

template<unsigned k, typename num, int h, int w>
constexpr num get(const Mat<num, h, w>& m) { return m(k); }

template<unsigned k, typename num, int h, int w>
constexpr num& get(Mat<num, h, w>& m) { return m(k); }

} // ns simple_mat

template<typename num, int h, int w>
using Mat = simple_mat::Mat<num, h, w>;

template<typename num, int h, int w>
constexpr Mat<num, h, w> operator*(num scalar, const Mat<num, h, w>& mat)
{
    return mat * scalar;
}

template<typename num, int H, int W>
constexpr Mat<num, H, W> operator*(const Mat<num, H, W>& self, num other)
{
    Mat<num, H, W> ret;
    for (int j = 0; j < H; j++)
        for (int i = 0; i < W; i++)
            ret(j, i) = self(j, i) * other;
    return ret;
}

namespace std {
    template<typename num, int H, int W>
    struct tuple_size<Mat<num, H, W>> :
        std::integral_constant<std::size_t, H == 1 || W == 1 ? W * H : 0>
    {};

    template<std::size_t k, typename num, int h, int w>
    struct tuple_element<k, Mat<num, h, w>>
    {
        using type = std::remove_const_t<std::remove_reference_t<num>>;
    };
} // ns std
