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
    const double dz_len_ = dz_len(),
                 expt_len_ = expt_len(),
                 expt_norm_ = expt_norm(),
                 expt_slope_ = expt_slope(),
                 log_len_ = log_len();

    const double expt_deriv_at_end = expt_norm_ * expt_slope_ * std::pow(expt_len_, expt_slope_ - 1); // cnx^(n-1)
    const double expt_at_end = expt_norm_ * std::pow(expt_len_, expt_slope_); // cx^n

    const double lin_len = 1 - dz_len_ - expt_len_ - log_len_;

    // this isn't correct but works.
    // we use exponentiation of the linear part to get logarithmic approximation of the linear
    // part rather than linear approximation of the linear part

    const double lin_at_end = std::pow(M_E, lin_len * expt_deriv_at_end - expt_at_end); // e^(cx + a)
    const double lin_deriv_at_end = expt_deriv_at_end * std::exp(-expt_at_end + expt_deriv_at_end * lin_len); // ce^(a + cx)

    // this was all derived by the awesome linear approximation
    // calculator <http://www.emathhelp.net/calculators/calculus-1/linear-approximation-calculator/>

    auto expt_part = [=](double x) { return expt_norm_ * std::pow(x, expt_slope_); };
    const double expt_inv_norm = expt_norm_/expt_part(expt_len_);

    auto lin_part = [=](double x) { return expt_inv_norm * (expt_at_end + expt_deriv_at_end * (x - expt_len_)); };
    const double lin_inv_norm = (1 - expt_norm_ - .25)/lin_part(lin_len);

    // TODO needs norm for log/lin parts
    auto log_part = [=](double x) { return expt_inv_norm * lin_inv_norm * std::log(lin_at_end + lin_deriv_at_end * (x - lin_len)); };
    const double log_inv_norm = .25/log_part(expt_len_);

    qDebug() << "lin" << expt_deriv_at_end << lin_inv_norm;

    part functors[]
    {
        { dz_len_, [](double) { return 0; } },
        { expt_len_, [=](double x) { return expt_inv_norm * expt_part(x); } }, // cx^n
        { lin_len, [=](double x) { return lin_inv_norm * lin_part(x); } }, // cx + a
        { log_len_, [=](double x) { return log_inv_norm * log_part(x); } }, // ln(cx + a)
    };

    make_spline_(functors, std::distance(std::begin(functors), std::end(functors)));
}

rel_settings::rel_settings() :
    opts("tobii-eyex-relative-mode"),
    speed(b, "speed", s(3, .1, 10)),
    dz_len(b, "deadzone-length", s(.04, 0, .2)),
    expt_slope(b, "exponent-slope", s(1.75, 1.5, 3)),
    expt_len(b, "exponent-length", s(.2, 0, .5)),
    expt_norm(b, "exponent-norm", s(.4, .1, .5)),
    log_len(b, "log-len", s(.1, 0, .2)),
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
    acc_mode_spline.clear();

    double lastx = 0;

    for (unsigned k = 0; k < len; k++)
    {
        part& fun = functors[k];

        static constexpr unsigned nparts = 7;

        for (unsigned i = 1; i <= nparts; i++)
        {
            const double x = i*fun.len/nparts;
            const double y = clamp(fun.f(x), 0, 1);
            if (i == nparts/2)
                qDebug() << k << i << x << y;
            acc_mode_spline.add_point((lastx + x) * spline_max, y * spline_max);
        }

        lastx += fun.len;
    }
}
