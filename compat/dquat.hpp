#pragma once
#include <cmath>

// To the extent possible under law, I dedicate this work to the public domain
// and waive all copyright and related rights worldwide. If this waiver is not
// legally effective in your jurisdiction, I grant an unconditional, perpetual,
// irrevocable, and royalty-free license to use this work for any purpose. No
// permission or attribution is required.

// formulas were taken from QQuaternion -sh

template<typename Float>
class Quat
{
    Float w = 1, x = 0, y = 0, z = 0;

    constexpr Quat(Float w, Float x, Float y, Float z) noexcept: w{w}, x{x}, y{y}, z{z} {}
    static Float clamp(Float x, Float min, Float max) { return x < min ? min : x > max ? max : x; }

    struct Vector { Float x, y, z; };

public:
    constexpr Quat() noexcept = default;
    constexpr Quat(const Quat&) noexcept = default;
    constexpr Quat<Float>& operator=(const Quat<Float>&) noexcept = default;

    constexpr Float lengthʹ() const { return w*w + x*x + y*y + z*z; }
    Float length() const { return std::sqrt(lengthʹ()); }
    constexpr Quat conjugated() const { return Quat<Float>{w, -x, -y, -z}; }
    friend constexpr Quat operator*(Float c, const Quat& q) noexcept { return q * c; }
    constexpr bool is_identity() const { return w == Float{1} && x == Float{0} && y == Float{0} && z == Float{0}; }

    void to_euler(Float& pitch, Float& yaw, Float& roll) const
    {
        //constexpr auto pi  = Float(3.141592653589793238462643383279502884);
        constexpr auto eps = Float{1e-6};
        constexpr auto r2d = Float(57.295779513082320876798154814105170336);

        Float inv_len;
        if (Float len = length(); len >= eps) [[likely]]
            inv_len = Float{1} / len;
        else
            inv_len = 1;

        Float xʹ = x * inv_len;
        Float yʹ = y * inv_len;
        Float zʹ = z * inv_len;
        Float wʹ = w * inv_len;

        Float xx = xʹ * xʹ;
        Float xy = xʹ * yʹ;
        Float xz = xʹ * zʹ;
        Float xw = xʹ * wʹ;
        Float yy = yʹ * yʹ;
        Float yz = yʹ * zʹ;
        Float yw = yʹ * wʹ;
        Float zz = zʹ * zʹ;
        Float zw = zʹ * wʹ;

        Float s = Float{-2.0} * (yz - xw);
        s = clamp(s, Float{-1}, Float{1});

#if 1
        pitch = r2d * std::asin(s);
        yaw   = r2d * std::atan2(Float{2} * (xz + yw), Float{1} - Float{2}  * (xx + yy));
        roll  = r2d * std::atan2(Float{2} * (xy + zw), Float{1} - Float{2} * (xx + zz));
#else
        if (std::fabs(s) < Float{1} - eps) [[likely]] {
            pitch = r2d * std::asin(s);
            yaw   = r2d * std::atan2(Float{2} * (xz + yw), Float{1} - Float{2} * (xx + yy));
            roll  = r2d * std::atan2(Float{2} * (xy + zw), Float{1} - Float{2} * (xx + zz));
        } else {
            pitch = r2d * std::copysign(pi, s);
            yaw   = r2d * Float{2} * std::atan2(yʹ, wʹ);
            roll  = r2d * Float{0};
        }
#endif
    }

    static Quat from_euler(Float pitch, Float yaw, Float roll);
    constexpr Vector rotate_point(Quat<Float>::Vector v) const;

    friend constexpr Quat operator*(const Quat& q, Float c) noexcept { return { q.w * c, q.x * c, q.y * c, q.z * c, }; }
    friend constexpr Quat operator*(const Quat<Float>& a, const Quat<Float>& b)
    {
        Float yʹ = (a.w - a.y) * (b.w + b.z);
        Float zʹ = (a.w + a.y) * (b.w - b.z);
        Float wʹ = (a.z + a.x) * (b.x + b.y);
        Float xʹ = wʹ + yʹ + zʹ;
        Float q = Float{0.5} * (xʹ + (a.z - a.x) * (b.x - b.y));

        Float w = q - wʹ + (a.z - a.y) * (b.y - b.z);
        Float x = q - xʹ + (a.x + a.w) * (b.x + b.w);
        Float y = q - yʹ + (a.w - a.x) * (b.y + b.z);
        Float z = q - zʹ + (a.z + a.y) * (b.w - b.x);

        return Quat<Float>(w, x, y, z);
    }
};

template<typename Float>
Quat<Float> Quat<Float>::from_euler(Float pitch, Float yaw, Float roll)
{
    constexpr auto d2r = Float(.017453292519943295769236907684886127);
    constexpr auto half = Float{0.5};
    constexpr auto c = d2r * half;

    pitch = c * pitch;
    yaw   = c * yaw;
    roll  = c * roll;

    Float c1 = std::cos(yaw),   s1 = std::sin(yaw);
    Float c2 = std::cos(roll),  s2 = std::sin(roll);
    Float c3 = std::cos(pitch), s3 = std::sin(pitch);
    Float c1c2 = c1 * c2;
    Float s1s2 = s1 * s2;
    Float w = c1c2 * c3 + s1s2 * s3;
    Float x = c1c2 * s3 + s1s2 * c3;
    Float y = s1 * c2 * c3 - c1 * s2 * s3;
    Float z = c1 * s2 * c3 - s1 * c2 * s3;

    return Quat{w, x, y, z};
}

template<typename Float>
constexpr Quat<Float>::Vector Quat<Float>::rotate_point(Vector v) const
{
    auto c = Float{1} / lengthʹ();
    auto q = *this * Quat{0, v.x, v.y, v.z} * conjugated();
    return { .x = q.x * c, .y = q.y * c, .z = q.z * c, };
}

using dquat = Quat<double>;
using fquat = Quat<float>;
