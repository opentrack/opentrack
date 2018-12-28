#pragma once

#include "simple-mat.hpp"
#include <array>
#include <vector>
#include <tuple>

#include "export.hpp"

namespace correlation_calibrator_impl {

static constexpr inline double min[6] = {
    -50,
    -50,
    250,

    -180,
    -180,
    -180,
};

static constexpr inline double max[6] = {
    50,
    50,
    250,

    180,
    180,
    180,
};

static constexpr inline double yaw_spacing_in_degrees = 1.5;
static constexpr inline double pitch_spacing_in_degrees = 1;
static constexpr inline double roll_spacing_in_degrees = 1;

static constexpr inline unsigned yaw_nbuckets   = unsigned(1+ 360./yaw_spacing_in_degrees);
static constexpr inline unsigned pitch_nbuckets = unsigned(1+ 360./pitch_spacing_in_degrees);
static constexpr inline unsigned roll_nbuckets  = unsigned(1+ 360./roll_spacing_in_degrees);

static constexpr inline double translation_spacing = .25;
static constexpr inline unsigned x_nbuckets = unsigned(1+ (max[0]-min[0])/translation_spacing);
static constexpr inline unsigned y_nbuckets = unsigned(1+ (max[1]-min[1])/translation_spacing);
static constexpr inline unsigned z_nbuckets = unsigned(1+ (max[2]-min[2])/translation_spacing);

using vec6 = Mat<double, 6, 1>;
using mat66 = Mat<double, 6, 6>;

class OTR_COMPAT_EXPORT correlation_calibrator final
{
    // careful to avoid vector copies
    std::array<std::vector<bool>, 6> buckets =
    {
        std::vector<bool>(x_nbuckets,       false),
        std::vector<bool>(y_nbuckets,       false),
        std::vector<bool>(z_nbuckets,       false),
        std::vector<bool>(yaw_nbuckets,     false),
        std::vector<bool>(pitch_nbuckets,   false),
        std::vector<bool>(roll_nbuckets,    false),
    };

    std::vector<vec6> data;

    bool check_buckets(const vec6& data);

public:
    correlation_calibrator() = default;
    void input(const vec6& data);
    mat66 get_coefficients() const;
    unsigned sample_count() const;

    static constexpr inline unsigned min_samples = 25;
};

} // ns correlation_calibrator_impl

using correlation_calibrator = correlation_calibrator_impl::correlation_calibrator;
