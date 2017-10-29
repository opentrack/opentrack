/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "point_tracker.h"
#include "compat/nan.hpp"
#include "compat/math-imports.hpp"

using namespace types;

#include <vector>
#include <algorithm>
#include <cmath>

#include <QDebug>

constexpr unsigned PointModel::N_POINTS;

static void get_row(const mat33& m, int i, vec3& v)
{
    v[0] = m(i,0);
    v[1] = m(i,1);
    v[2] = m(i,2);
}

static void set_row(mat33& m, int i, const vec3& v)
{
    m(i,0) = v[0];
    m(i,1) = v[1];
    m(i,2) = v[2];
}

PointModel::PointModel(const settings_pt& s)
{
    set_model(s);
    // calculate u
    u = M01.cross(M02);
    u = cv::normalize(u);

    // calculate projection matrix on M01,M02 plane
    f s11 = M01.dot(M01);
    f s12 = M01.dot(M02);
    f s22 = M02.dot(M02);
    P = 1/(s11*s22-s12*s12) * mat22(s22, -s12, -s12,  s11);
}

void PointModel::set_model(const settings_pt& s)
{
    switch (s.active_model_panel)
    {
    case Clip:
        M01 = vec3(0, static_cast<f>(s.clip_ty), -static_cast<f>(s.clip_tz));
        M02 = vec3(0, -static_cast<f>(s.clip_by), -static_cast<f>(s.clip_bz));
        break;
    case Cap:
        M01 = vec3(-static_cast<f>(s.cap_x), -static_cast<f>(s.cap_y), -static_cast<f>(s.cap_z));
        M02 = vec3(static_cast<f>(s.cap_x), -static_cast<f>(s.cap_y), -static_cast<f>(s.cap_z));
        break;
    case Custom:
        M01 = vec3(s.m01_x, s.m01_y, s.m01_z);
        M02 = vec3(s.m02_x, s.m02_y, s.m02_z);
        break;
    }
}

void PointModel::get_d_order(const vec2* points, unsigned* d_order, const vec2& d) const
{
    // fit line to orthographically projected points
    using t = std::pair<f,unsigned>;
    t d_vals[3];
    // get sort indices with respect to d scalar product
    for (unsigned i = 0; i < PointModel::N_POINTS; ++i)
        d_vals[i] = t(d.dot(points[i]), i);

    std::sort(d_vals,
              d_vals + 3u,
              [](const t& a, const t& b) { return a.first < b.first; });

    for (unsigned i = 0; i < PointModel::N_POINTS; ++i)
        d_order[i] = d_vals[i].second;
}


PointTracker::PointTracker() : init_phase(true), prev_order_valid(false)
{
}

PointTracker::PointOrder PointTracker::find_correspondences_previous(const vec2* points,
                                                                     const PointModel& model,
                                                                     const CamInfo& info)
{
    const double fx = info.get_focal_length();
    PointTracker::PointOrder p;
    p[0] = project(vec3(0,0,0), fx);
    p[1] = project(model.M01, fx);
    p[2] = project(model.M02, fx);

    const int diagonal = int(std::sqrt(f(info.res_x*info.res_x + info.res_y*info.res_y)));
    constexpr int div = 100;
    const int max_dist = diagonal / div; // 8 pixels for 640x480

    // set correspondences by minimum distance to projected model point
    bool point_taken[PointModel::N_POINTS];
    for (unsigned i=0; i<PointModel::N_POINTS; ++i)
        point_taken[i] = false;

    for (unsigned i=0; i<PointModel::N_POINTS; ++i)
    {
        f min_sdist = 0;
        unsigned min_idx = 0;
        // find closest point to projected model point i
        for (unsigned j=0; j<PointModel::N_POINTS; ++j)
        {
            vec2 d = p[i]-points[j];
            f sdist = d.dot(d);
            if (sdist < min_sdist || j==0)
            {
                min_idx = j;
                min_sdist = sdist;
            }
        }
        if (min_sdist > max_dist)
            return find_correspondences(points, model);

        // if one point is closest to more than one model point, fallback
        if (point_taken[min_idx])
        {
            init_phase = true;
            return find_correspondences(points, model);
        }
        point_taken[min_idx] = true;
        p[i] = points[min_idx];
    }

    return p;
}

bool PointTracker::maybe_use_old_point_order(const PointOrder& order, const CamInfo& info)
{
    constexpr f std_width = 640, std_height = 480;

    PointOrder scaled_order;

    const f cx = std_width / info.res_x;
    const f cy = std_height / info.res_y;

    for (unsigned k = 0; k < 3; k++)
    {
        // note, the .y component is actually scaled by width
        scaled_order[k][0] = std_width * cx * order[k][0];
        scaled_order[k][1] = std_width * cy * order[k][1];
    }

    f sum = 0;

    for (unsigned k = 0; k < 3; k++)
    {
        vec2 tmp = prev_scaled_order[k] - scaled_order[k];
        sum += std::sqrt(tmp.dot(tmp));
    }

    // CAVEAT don't increase too much, it visibly loses precision
    constexpr f max_dist = f(.13);

    const bool validp = sum < max_dist;

    prev_order_valid &= validp;

    if (!prev_order_valid)
    {
        prev_order = order;
        prev_scaled_order = scaled_order;
    }

#if 0
    {
        static Timer tt;
        static int cnt1 = 0, cnt2 = 0;
        if (tt.elapsed_ms() >= 1000)
        {
            tt.start();
            if (cnt1 + cnt2)
            {
                qDebug() << "old-order" << ((cnt1 * 100) / f(cnt1 + cnt2)) << "nsamples" << (cnt1 + cnt2);
                cnt1 = 0, cnt2 = 0;
            }
        }
        if (validp)
            cnt1++;
        else
            cnt2++;
    }
#endif

    prev_order_valid = validp;

    return validp;
}

void PointTracker::track(const std::vector<vec2>& points,
                         const PointModel& model,
                         const CamInfo& info,
                         int init_phase_timeout)
{
    const double fx = info.get_focal_length();
    PointOrder order;

    if (init_phase_timeout > 0 && t.elapsed_ms() > init_phase_timeout)
    {
        t.start();
        init_phase = true;
    }

    if (!(init_phase_timeout > 0 && !init_phase))
        order = find_correspondences(points.data(), model);
    else
        order = find_correspondences_previous(points.data(), model, info);

    if (maybe_use_old_point_order(order, info) ||
        POSIT(model, order, fx) != -1)
    {
        init_phase = false;
        t.start();
    }
}

PointTracker::PointOrder PointTracker::find_correspondences(const vec2* points, const PointModel& model)
{
    static const Affine a(mat33::eye(), vec3(0, 0, 1));
    // We do a simple freetrack-like sorting in the init phase...
    unsigned point_d_order[PointModel::N_POINTS];
    unsigned model_d_order[PointModel::N_POINTS];
    // sort points
    vec2 d(model.M01[0]-model.M02[0], model.M01[1]-model.M02[1]);
    model.get_d_order(points, point_d_order, d);
    // calculate d and d_order for simple freetrack-like point correspondence
    vec2 pts[3] = {
        vec2(0, 0),
        vec2(model.M01[0], model.M01[1]),
        vec2(model.M02[0], model.M02[1])
    };
    model.get_d_order(pts, model_d_order, d);
    // set correspondences
    PointOrder p;
    for (unsigned i = 0; i < PointModel::N_POINTS; ++i)
        p[model_d_order[i]] = points[point_d_order[i]];

    return p;
}

int PointTracker::POSIT(const PointModel& model, const PointOrder& order, f focal_length)
{
    // POSIT algorithm for coplanar points as presented in
    // [Denis Oberkampf, Daniel F. DeMenthon, Larry S. Davis: "Iterative Pose Estimation Using Coplanar Feature Points"]
    // we use the same notation as in the paper here

    // The expected rotation used for resolving the ambiguity in POSIT:
    // In every iteration step the rotation closer to R_expected is taken
    static const mat33 R_expected(X_CM.R);

    // initial pose = last (predicted) pose
    vec3 k;
    get_row(R_expected, 2, k);
    f Z0 = X_CM.t[2] < f(1e-4) ? f(1000) : X_CM.t[2];

    f old_epsilon_1 = 0;
    f old_epsilon_2 = 0;
    f epsilon_1 = 1;
    f epsilon_2 = 1;

    vec3 I0, J0;
    vec2 I0_coeff, J0_coeff;

    vec3 I_1, J_1, I_2, J_2;
    mat33 R_1, R_2;
    mat33* R_current = &R_1;

    constexpr int max_iter = 100;

    int i=1;
    for (; i<max_iter; ++i)
    {
        epsilon_1 = k.dot(model.M01)/Z0;
        epsilon_2 = k.dot(model.M02)/Z0;

        // vector of scalar products <I0, M0i> and <J0, M0i>
        vec2 I0_M0i(order[1][0]*(1 + epsilon_1) - order[0][0],
                order[2][0]*(1 + epsilon_2) - order[0][0]);
        vec2 J0_M0i(order[1][1]*(1 + epsilon_1) - order[0][1],
                order[2][1]*(1 + epsilon_2) - order[0][1]);

        // construct projection of I, J onto M0i plane: I0 and J0
        I0_coeff = model.P * I0_M0i;
        J0_coeff = model.P * J0_M0i;
        I0 = I0_coeff[0]*model.M01 + I0_coeff[1]*model.M02;
        J0 = J0_coeff[0]*model.M01 + J0_coeff[1]*model.M02;

        // calculate u component of I, J
        f II0 = I0.dot(I0);
        f IJ0 = I0.dot(J0);
        f JJ0 = J0.dot(J0);
        f rho, theta;
        // CAVEAT don't change to comparison with an epsilon -sh 20160423
        if (JJ0 == II0) {
            rho = sqrt(fabs(2*IJ0));
            theta = -M_PI/4;
            if (IJ0<0) theta *= -1;
        }
        else {
            rho = sqrt(sqrt( (JJ0-II0)*(JJ0-II0) + 4*IJ0*IJ0 ));
            theta = atan( -2*IJ0 / (JJ0-II0) );
            // avoid branch misprediction
            theta += (JJ0 - II0 < 0) * M_PI;
            theta *= f(.5);
        }

        // construct the two solutions
        I_1 = I0 + rho*cos(theta)*model.u;
        I_2 = I0 - rho*cos(theta)*model.u;

        J_1 = J0 + rho*sin(theta)*model.u;
        J_2 = J0 - rho*sin(theta)*model.u;

        f norm_const = 1/cv::norm(I_1); // all have the same norm

        // create rotation matrices
        I_1 *= norm_const; J_1 *= norm_const;
        I_2 *= norm_const; J_2 *= norm_const;

        set_row(R_1, 0, I_1);
        set_row(R_1, 1, J_1);
        set_row(R_1, 2, I_1.cross(J_1));

        set_row(R_2, 0, I_2);
        set_row(R_2, 1, J_2);
        set_row(R_2, 2, I_2.cross(J_2));

        // the single translation solution
        Z0 = norm_const * focal_length;

        // pick the rotation solution closer to the expected one
        // in simple metric d(A,B) = || I - A * B^T ||
        f R_1_deviation = cv::norm(mat33::eye() - R_expected * R_1.t());
        f R_2_deviation = cv::norm(mat33::eye() - R_expected * R_2.t());

        if (R_1_deviation < R_2_deviation)
            R_current = &R_1;
        else
            R_current = &R_2;

        get_row(*R_current, 2, k);

        // check for convergence condition
        const f delta = fabs(epsilon_1 - old_epsilon_1) + fabs(epsilon_2 - old_epsilon_2);

        if (!(delta > constants::eps))
            break;

        old_epsilon_1 = epsilon_1;
        old_epsilon_2 = epsilon_2;
    }

    const f t[3] = {
        order[0][0] * Z0/focal_length,
        order[0][1] * Z0/focal_length,
        Z0
    };
    const mat33& r = *R_current;

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (nanp(r(i, j)))
            {
                qDebug() << "posit nan";
                return -1;
            }

    for (unsigned i = 0; i < 3; i++)
        if (nanp(t[i]))
        {
            qDebug() << "posit nan";
            return -1;
        }

    // apply results
    X_CM.R = r;
    X_CM.t[0] = t[0];
    X_CM.t[1] = t[1];
    X_CM.t[2] = t[2];

    //qDebug() << "iter:" << i;

    return i;
}

vec2 PointTracker::project(const vec3& v_M, f focal_length)
{
    return project(v_M, focal_length, X_CM);
}

vec2 PointTracker::project(const vec3& v_M, f focal_length, const Affine& X_CM)
{
    vec3 v_C = X_CM * v_M;
    return vec2(focal_length*v_C[0]/v_C[2], focal_length*v_C[1]/v_C[2]);
}

