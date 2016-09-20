/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "point_tracker.h"

#include <vector>
#include <algorithm>
#include <cmath>

#include <QDebug>

static void get_row(const cv::Matx33d& m, int i, cv::Vec3d& v)
{
    v[0] = m(i,0);
    v[1] = m(i,1);
    v[2] = m(i,2);
}

static void set_row(cv::Matx33d& m, int i, const cv::Vec3d& v)
{
    m(i,0) = v[0];
    m(i,1) = v[1];
    m(i,2) = v[2];
}

static bool d_vals_sort(const std::pair<double,int> a, const std::pair<double,int> b)
{
    return a.first < b.first;
}

void PointModel::get_d_order(const std::vector<cv::Vec2d>& points, int* d_order, const cv::Vec2d& d) const
{
    // fit line to orthographically projected points
    std::vector<std::pair<double,int>> d_vals;
    // get sort indices with respect to d scalar product
    for (unsigned i = 0; i < N_POINTS; ++i)
        d_vals.push_back(std::pair<double, int>(d.dot(points[i]), i));

    std::sort(d_vals.begin(),
              d_vals.end(),
              d_vals_sort
              );

    for (unsigned i = 0; i<points.size(); ++i)
        d_order[i] = d_vals[i].second;
}


PointTracker::PointTracker() : init_phase(true)
{
}

PointTracker::PointOrder PointTracker::find_correspondences_previous(const std::vector<cv::Vec2d>& points,
                                                                     const PointModel& model,
                                                                     double focal_length,
                                                                     int w,
                                                                     int h)
{
    PointTracker::PointOrder p;
    p.points[0] = project(cv::Vec3d(0,0,0), focal_length);
    p.points[1] = project(model.M01, focal_length);
    p.points[2] = project(model.M02, focal_length);

    const int diagonal = int(std::sqrt(w*w + h*h));
    static constexpr int div = 100;
    const int max_dist = diagonal / div; // 8 pixels for 640x480

    // set correspondences by minimum distance to projected model point
    bool point_taken[PointModel::N_POINTS];
    for (int i=0; i<PointModel::N_POINTS; ++i)
        point_taken[i] = false;

    for (int i=0; i<PointModel::N_POINTS; ++i)
    {
        double min_sdist = 0;
        unsigned min_idx = 0;
        // find closest point to projected model point i
        for (unsigned j=0; j<PointModel::N_POINTS; ++j)
        {
            cv::Vec2d d = p.points[i]-points[j];
            double sdist = d.dot(d);
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
        p.points[i] = points[min_idx];
    }
    return p;
}

void PointTracker::track(const std::vector<cv::Vec2d>& points,
                         const PointModel& model,
                         double focal_length,
                         bool dynamic_pose,
                         int init_phase_timeout,
                         int w,
                         int h)
{
    PointOrder order;

    if (t.elapsed_ms() > init_phase_timeout)
    {
        t.start();
        init_phase = true;
    }

    if (!dynamic_pose || init_phase)
        order = find_correspondences(points, model);
    else
    {
        order = find_correspondences_previous(points, model, focal_length, w, h);
    }

    POSIT(model, order, focal_length);
    init_phase = false;
    t.start();
}

PointTracker::PointOrder PointTracker::find_correspondences(const std::vector<cv::Vec2d>& points, const PointModel& model)
{
    // We do a simple freetrack-like sorting in the init phase...
    // sort points
    int point_d_order[PointModel::N_POINTS];
    int model_d_order[PointModel::N_POINTS];
    cv::Vec2d d(model.M01[0]-model.M02[0], model.M01[1]-model.M02[1]);
    model.get_d_order(points, point_d_order, d);
    // calculate d and d_order for simple freetrack-like point correspondence
    model.get_d_order(std::vector<cv::Vec2d> {
                          cv::Vec2d{0,0},
                          cv::Vec2d(model.M01[0], model.M01[1]),
                          cv::Vec2d(model.M02[0], model.M02[1])
                      },
                      model_d_order,
                      d);
    // set correspondences
    PointOrder p;
    for (int i=0; i<PointModel::N_POINTS; ++i)
        p.points[model_d_order[i]] = points[point_d_order[i]];

    return p;
}

template<typename t, int h, int w>
bool nanp(const cv::Matx<t, h, w>& m)
{
    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++)
            if (nanp(m(i, j)))
                return true;
    return false;
}

bool PointTracker::POSIT(const PointModel& model, const PointOrder& order_, double focal_length)
{
    // POSIT algorithm for coplanar points as presented in
    // [Denis Oberkampf, Daniel F. DeMenthon, Larry S. Davis: "Iterative Pose Estimation Using Coplanar Feature Points"]
    // we use the same notation as in the paper here

    // The expected rotation used for resolving the ambiguity in POSIT:
    // In every iteration step the rotation closer to R_expected is taken
    cv::Matx33d R_expected = cv::Matx33d::eye();

    // initial pose = last (predicted) pose
    cv::Vec3d k;
    get_row(R_expected, 2, k);

    double Z0 = 1000;
    double old_epsilon_1 = 0;
    double old_epsilon_2 = 0;
    double epsilon_1 = 1;
    double epsilon_2 = 1;

    cv::Vec3d I0, J0;
    cv::Vec2d I0_coeff, J0_coeff;

    cv::Vec3d I_1, J_1, I_2, J_2;
    cv::Matx33d R_1, R_2;
    cv::Matx33d& R_current = R_1;

    const int MAX_ITER = 500;
    static constexpr double eps = 1e-6;

    const cv::Vec2d* order = order_.points;
    using std::sqrt;
    using std::atan;
    using std::cos;
    using std::sin;
    using std::fabs;

    int i=1;
    for (; i<MAX_ITER; ++i)
    {
        epsilon_1 = k.dot(model.M01)/Z0;
        epsilon_2 = k.dot(model.M02)/Z0;

        // vector of scalar products <I0, M0i> and <J0, M0i>
        cv::Vec2d I0_M0i(order[1][0]*(1.0 + epsilon_1) - order[0][0],
                order[2][0]*(1.0 + epsilon_2) - order[0][0]);
        cv::Vec2d J0_M0i(order[1][1]*(1.0 + epsilon_1) - order[0][1],
                order[2][1]*(1.0 + epsilon_2) - order[0][1]);

        // construct projection of I, J onto M0i plane: I0 and J0
        I0_coeff = model.P * I0_M0i;
        J0_coeff = model.P * J0_M0i;
        I0 = I0_coeff[0]*model.M01 + I0_coeff[1]*model.M02;
        J0 = J0_coeff[0]*model.M01 + J0_coeff[1]*model.M02;

        // calculate u component of I, J
        double II0 = I0.dot(I0);
        double IJ0 = I0.dot(J0);
        double JJ0 = J0.dot(J0);
        double rho, theta;
        if (JJ0 == II0) {
            rho = std::sqrt(std::fabs(2*IJ0));
            theta = -M_PI/4;
            if (IJ0<0) theta *= -1;
        }
        else {
            rho = sqrt(sqrt( (JJ0-II0)*(JJ0-II0) + 4*IJ0*IJ0 ));
            theta = atan( -2*IJ0 / (JJ0-II0) );
            // avoid branch misprediction
            theta += (JJ0 - II0 < 0) * M_PI;
            theta /= 2;
        }

        // construct the two solutions
        I_1 = I0 + rho*cos(theta)*model.u;
        I_2 = I0 - rho*cos(theta)*model.u;

        J_1 = J0 + rho*sin(theta)*model.u;
        J_2 = J0 - rho*sin(theta)*model.u;

        double norm_const = 1/cv::norm(I_1); // all have the same norm

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
        double R_1_deviation = cv::norm(cv::Matx33d::eye() - R_expected * R_1.t());
        double R_2_deviation = cv::norm(cv::Matx33d::eye() - R_expected * R_2.t());

        if (R_1_deviation < R_2_deviation)
            R_current = R_1;
        else
            R_current = R_2;

        get_row(R_current, 2, k);

        // check for convergence condition
        const double delta = fabs(epsilon_1 - old_epsilon_1) + fabs(epsilon_2 - old_epsilon_2);

        if (!(delta > eps))
            break;

        old_epsilon_1 = epsilon_1;
        old_epsilon_2 = epsilon_2;
    }

    // apply results
    X_CM.R = R_current;
    X_CM.t[0] = order[0][0] * Z0/focal_length;
    X_CM.t[1] = order[0][1] * Z0/focal_length;
    X_CM.t[2] = Z0;

    //qDebug() << "iter:" << i;

    return i;
}

cv::Vec2d PointTracker::project(const cv::Vec3d& v_M, double f)
{
    cv::Vec3d v_C = X_CM * v_M;
    return cv::Vec2d(f*v_C[0]/v_C[2], f*v_C[1]/v_C[2]);
}
