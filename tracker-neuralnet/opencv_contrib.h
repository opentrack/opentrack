#pragma once

#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/quaternion.hpp>

// Well eventually it might be a contribution

namespace cvcontrib
{

template <class T> cv::Point_<T> as_point(const cv::Size_<T>& s)
{
    return { s.width, s.height };
}

template <class T> cv::Size_<T> as_size(const cv::Point_<T>& p)
{
    return { p.x, p.y };
}

template <int n, int m> inline bool allfinite(const cv::Matx<float, n, m>& mat)
{
    const size_t sz = mat.rows * mat.cols;
    for (size_t i = 0; i < sz; ++i)
        if (!std::isfinite(mat.val[i]))
            return false;
    return true;
}

// Because compiler refuses to convert it automatically
template <int n> inline cv::Vec<float, n> to_vec(const cv::Matx<float, n, 1>& m)
{
    return cv::Vec<float, n>{ m.val };
}

template <int n, int m, int o> inline void set_minor(cv::Vec<float, m>& dst, const int startrow, const cv::Matx<float, o, 1>& src)
{
    assert(startrow >= 0 && startrow + n <= dst.rows);
    for (int row = startrow, i = 0; row < startrow + n; ++row, ++i)
    {
        dst[row] = src(i, 0);
    }
}

template <int nrows, int ncols, int m, int n>
inline void set_minor(cv::Matx<float, m, n>& dst, const int startrow, int startcol, const cv::Matx<float, nrows, ncols>& src)
{
    assert(startrow >= 0 && startrow + nrows <= dst.rows);
    assert(startcol >= 0 && startcol + ncols <= dst.cols);
    for (int row = startrow, i = 0; row < startrow + nrows; ++row, ++i)
    {
        for (int col = startcol, j = 0; col < startcol + ncols; ++col, ++j)
        {
            dst(row, col) = src(i, j);
        }
    }
}

inline cv::Quatf identity_quat()
{
    return cv::Quatf(1, 0, 0, 0);
}

inline cv::Vec3f toRotVec(const cv::Quatf& q)
{
    // This is an improved implementation
#if 1
    // w = cos(alpha/2)
    // xyz = sin(alpha/2)*axis
    static constexpr float eps = 1.e-12f;
    const cv::Vec3f xyz{ q.x, q.y, q.z };
    const float len = cv::norm(xyz);
    const float angle = std::atan2(len, q.w) * 2.f;
    return xyz * (angle / (len + eps));
#else
    // The opencv implementation fails even the simplest test:
    // out = toRVec(cv::Quatf{1., 0., 0., 0. });
    // ASSERT_TRUE(std::isfinite(out[0]) && std::isfinite(out[1]) && std::isfinite(out[2]));
    return q.toRotVec();
#endif
}

inline cv::Vec3f rotate(const cv::Quatf& q, const cv::Vec3f& v)
{
    const auto r = q * cv::Quatf{ 0., v[0], v[1], v[2] } * q.conjugate();
    return { r.x, r.y, r.z };
}

template <int n> inline cv::Matx<float, n, n> cholesky(const cv::Matx<float, n, n>& mat)
{
    cv::Matx<float, n, n> l = mat;
    // Der Code ist die Doku!
    // https://github.com/opencv/opencv/blob/4.5.4/modules/core/src/matrix_decomp.cpp#L95
    cv::Cholesky(l.val, l.cols * sizeof(float), n, nullptr, 0, 0);
    // It doesn't clear the upper triangle so we do it for it.
    for (int row = 0; row < n; ++row)
        for (int col = row + 1; col < n; ++col)
            l(row, col) = 0.f;
    return l;
}

} // namespace cvcontrib