#pragma once

#include "api/plugin-api.hpp"
#include "options/options.hpp"
using namespace options;
#include "spline/spline.hpp"
#include "spline/spline-widget.hpp"

#include <functional>

#include <QObject>

enum tobii_mode
{
    tobii_relative,
    tobii_absolute,
};

class rel_settings final : public QObject, public opts
{
    Q_OBJECT

    using functor = std::function<double(double)>;

    struct part
    {
        double len;
        functor f;
    };

    void make_spline_(part* functors, unsigned len);

public:
    using s = slider_value;
    value<slider_value> speed, dz_len, expt_slope, expt_len, expt_norm, log_len;
    spline acc_mode_spline;
    rel_settings();
    double gain(double value);

public slots:
    void make_spline();
};

struct settings final : public opts
{
    value<tobii_mode> mode;
    settings() :
        opts("tobii-eyex"),
        mode(b, "mode", tobii_relative)
    {}
};
