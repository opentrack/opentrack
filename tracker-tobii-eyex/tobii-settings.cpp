#include "tobii-settings.hpp"
#include <cmath>
#include <iterator>
#include <utility>
#include <numeric>

/*
    def plot(f, max=None, min=None):
      if max is None and min is None:
        min, max = -1.5, 1.5
      elif max is None:
        max=-min
      elif min is None:
        min=-max

      assert max > min
      c = 1e-4*(max-min)
      if c < 1e-12:
        c = 1e-6
      rng = arange(min, max, c)
      plt.plot(rng, map(f, rng))
*/

/*
    def richards(b, q, v, c=1):
        return lambda x: 1./((c + q * exp(-b * x) ** (1./v)))
*/

void rel_settings::make_spline()
{
    const double log_c = 1./std::log(log_slope());

    part functors[]
    {
        { 1, dz_len(), 0, [](double) { return 0; } },
        { 5, expt_len(), expt_norm(), [=](double x) { return std::pow(x, expt_slope()); } },
        { 7, 1 - dz_len() - expt_len() - log_len(), std::max(0., 1 - expt_norm() - log_norm()), [](double x) { return x; } },
        { 7, log_len(), log_norm(), [=](double x) { return std::log(1+x)*log_c; } },
    };

    make_spline_(functors, std::distance(std::begin(functors), std::end(functors)));
}

rel_settings::rel_settings() :
    opts("tobii-eyex-relative-mode"),
    speed(b, "speed", s(3, .1, 10)),
    dz_len(b, "deadzone-length", s(.04, 0, .2)),
    expt_slope(b, "exponent-slope", s(1.75, 1.25, 3)),
    expt_len(b, "exponent-length", s(.25, 0, .5)),
    expt_norm(b, "exponent-norm", s(.3, .1, .5)),
    log_slope(b, "log-slope", s(2.75, 1.25, 10)),
    log_len(b, "log-len", s(.1, 0, .2)),
    log_norm(b, "log-norm", s(.1, .05, .3)),
    acc_mode_spline(100, 100, "")
{
    make_spline();
}

// there's an underflow in spline code, can't use 1e0
static constexpr const double spline_max = 1e2;

double rel_settings::gain(double value)
{
    return acc_mode_spline.get_value_no_save(value * spline_max) / spline_max;
}

void rel_settings::make_spline_(part* functors, unsigned len)
{
    acc_mode_spline.removeAllPoints();

    double lastx = 0, lasty = 0;

    using std::accumulate;

    const double inv_norm_y = 1./accumulate(functors, functors + len, 1e-4, [](double acc, const part& functor) { return acc + functor.norm; });
    const double inv_norm_x = 1./accumulate(functors, functors + len, 1e-4, [](double acc, const part& functor) { return acc + functor.len; });

    for (unsigned k = 0; k < len; k++)
    {
        part& fun = functors[k];

        const double xscale = fun.len * spline_max * inv_norm_x;
        const double maxx = fun.f(1);
        const double yscale = fun.norm * spline_max * inv_norm_y * (maxx < 1e-3 ? 0 : 1./maxx);

        for (unsigned i = 0; i <= fun.nparts; i++)
        {
            const double x = lastx + (fun.nparts == 0 ? 1 : i) / (1.+fun.nparts) * xscale;
            const double y = lasty + clamp(fun.f(x) * yscale, 0, spline_max);
            qDebug() << k << i << x << y;
            acc_mode_spline.addPoint(x, y);
        }

        lastx += xscale;
        lasty += yscale;
    }
}
