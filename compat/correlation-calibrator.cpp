#include "correlation-calibrator.hpp"
#include "variance.hpp"
#include "compat/math.hpp"
#include "compat/meta.hpp"
#include "compat/macros.h"

#include <cmath>
#include <iterator>

#include <QDebug>

#define DEBUG_PRINT
#ifdef DEBUG_PRINT
#   include <cstdio>
#endif

namespace correlation_calibrator_impl {

static constexpr unsigned nbuckets[6] =
{
    x_nbuckets,
    y_nbuckets,
    z_nbuckets,

    yaw_nbuckets,
    pitch_nbuckets,
    roll_nbuckets,
};

static constexpr double spacing[6] =
{
    translation_spacing,
    translation_spacing,
    translation_spacing,

    yaw_spacing_in_degrees,
    pitch_spacing_in_degrees,
    roll_spacing_in_degrees,
};

static constexpr char const* const names[6] {
    "x", "y", "z",
    "yaw", "pitch", "roll",
};

bool correlation_calibrator::check_buckets(const vec6& data)
{
    bool ret = false;
    unsigned pos[6];

    for (unsigned k = 0; k < 6; k++)
    {
        const double val = std::clamp(data[k], min[k], max[k]);
        pos[k] = unsigned((val-min[k])/spacing[k]);

        if (pos[k] >= nbuckets[k])
        {
            eval_once(qDebug() << "idx" << k
                               << "bucket" << (int)pos[k]
                               << "outside bounds" << nbuckets[k]);

            return false;
        }

        if (!buckets[k][pos[k]])
        {
            ret = true;
            buckets[k][pos[k]] = true;
        }
    }

    return ret;
}

void correlation_calibrator::input(const vec6& data_)
{
    if (!check_buckets(data_))
        return;

    data.push_back(data_);
}

mat66 correlation_calibrator::get_coefficients() const
{
    if (data.size() < min_samples)
    {
        qDebug() << "correlation-calibrator: not enough data";

        mat66 ret;
        for (unsigned k = 0; k < 6; k++)
            ret(k, k) = 1;
        return ret;
    }

    variance vs[6];
    vec6 devs, means;

    for (const vec6& x : data)
        for (unsigned i = 0; i < 6; i++)
            vs[i].input(x(i));

    for (unsigned i = 0; i < 6; i++)
    {
        means(i) = vs[i].avg();
        devs(i) = vs[i].stddev();

        constexpr double EPS = 1e-4;

        if (devs(i) < EPS)
            devs(i) = EPS;
    }

    mat66 cs;

    for (const vec6& x : data)
        for (unsigned k = 0; k < 6; k++)
        {
            for (unsigned idx = 0; idx < 6; idx++)
            {
                const double zi = (x(idx) - means(idx)),
                             zk = (x(k) - means(k));

                cs(idx, k) += zi * zk / (devs(k)*devs(k));
            }
        }

    cs = cs * (1./(data.size() - 1));

#if defined DEBUG_PRINT
    fprintf(stderr, "v:change-of h:due-to\n");
    fprintf(stderr, "%10s ", "");
    for (char const* k : names)
        fprintf(stderr, "%10s", k);
    fprintf(stderr, "\n");

    for (unsigned i = 0; i < 6; i++)
    {
        fprintf(stderr, "%10s ", names[i]);
        for (unsigned k = 0; k < 6; k++)
            fprintf(stderr, "%10.3f", cs(i, k));
        fprintf(stderr, "\n");
    }
    fflush(stderr);
#endif

    for (unsigned k = 0; k < 6; k++)
        cs(k, k) = 1;

    // derivations from
    // https://www.thoughtco.com/how-to-calculate-the-correlation-coefficient-3126228

    return cs;
}

unsigned correlation_calibrator::sample_count() const
{
    return (unsigned)data.size();
}

} // ns correlation_calibrator_impl
