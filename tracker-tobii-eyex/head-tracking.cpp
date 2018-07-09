#include "tobii-eyex.hpp"
#include "compat/math.hpp"
#include "compat/math-imports.hpp"

real tobii_eyex_tracker::gain(real x)
{
    // simple sigmoid
    x = clamp(x * 12 - 6, -6, 6);
    x = 1 / (1 + exp(-x));
    x = x * 2 - 1;
    return clamp(x, -1, 1);
}

void tobii_eyex_tracker::data(double* data)
{
    real px, py, max_x, max_y;
    bool fresh;

    {
        QMutexLocker l(&global_state_mtx);

        if (!dev_state.is_valid())
            return;

        px = dev_state.px;
        py = dev_state.py;
        max_x = dev_state.display_res_x - 1;
        max_y = dev_state.display_res_y - 1;

        fresh = dev_state.fresh;
        dev_state.fresh = false;
    }

    if (fresh)
    {
        real x = (2*px - max_x)/max_x; // (-1)->1
        real y = (2*py - max_y)/max_y; // idem

        data[TX] = x * 50;
        data[TY] = y * -50;

        const double dt = t.elapsed_seconds();
        t.start();

        const double max_yaw = *s.acc_max_yaw, max_pitch = *s.acc_max_pitch;
        const double dz_ = *s.acc_dz; // closed set of 0->x, some arbitrary x < 1

        for (auto* k_ : { &x, &y })
        {
            real& k = *k_;

            if (std::fabs(k) < dz_)
                k = 0;
            else
            {
                // input has reduced dynamic range
                k -= copysign(dz_, k);
                k *= 1 + dz_;
            }
        }

        const double c = *s.acc_speed;

        // XXX check this for stability -sh 20180709
        const double yaw_delta = gain(x) * c * dt;
        const double pitch_delta = gain(y) * c * dt;

        yaw += yaw_delta;
        pitch += pitch_delta;

        yaw = clamp(yaw, -max_yaw, max_yaw);
        pitch = clamp(pitch, -max_pitch, max_pitch);
    }

    if (do_center)
    {
        do_center = false;
        yaw = 0;
        pitch = 0;
    }

    data[Yaw] = yaw;
    data[Pitch] = pitch;
    data[Roll] = 0;
    data[TZ] = 0; // XXX TODO

    // tan(x) in 0->.7 is almost linear. we don't need to adjust.
    // .7 is 40 degrees which is already quite a lot from the monitor.
}
