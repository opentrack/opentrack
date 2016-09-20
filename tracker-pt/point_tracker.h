/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef POINTTRACKER_H
#define POINTTRACKER_H

#include "compat/timer.hpp"

#include "ftnoir_tracker_pt_settings.h"
using namespace pt_types;

#include <opencv2/core/core.hpp>
#include <memory>
#include <vector>
#include <QObject>

class Affine final
{
public:
    Affine() : R(mat33::eye()), t(0,0,0) {}
    Affine(const mat33& R, const vec3& t) : R(R),t(t) {}

    mat33 R;
    vec3 t;
};

inline Affine operator*(const Affine& X, const Affine& Y)
{
    return Affine(X.R*Y.R, X.R*Y.t + X.t);
}

inline Affine operator*(const mat33& X, const Affine& Y)
{
    return Affine(X*Y.R, X*Y.t);
}

inline Affine operator*(const Affine& X, const mat33& Y)
{
    return Affine(X.R*Y, X.t);
}

inline vec3 operator*(const Affine& X, const vec3& v)
{
    return X.R*v + X.t;
}

// ----------------------------------------------------------------------------
// Describes a 3-point model
// nomenclature as in
// [Denis Oberkampf, Daniel F. DeMenthon, Larry S. Davis: "Iterative Pose Estimation Using Coplanar Feature Points"]
class PointModel final
{
    friend class PointTracker;
public:
    static constexpr unsigned N_POINTS = 3;

    vec3 M01;      // M01 in model frame
    vec3 M02;      // M02 in model frame

    vec3 u;        // unit vector perpendicular to M01,M02-plane

    mat22 P;

    enum Model { Clip, Cap, Custom };

    PointModel(settings_pt& s);
    void set_model(settings_pt& s);
    void get_d_order(const std::vector<vec2>& points, int* d_order, const vec2& d) const;
};

// ----------------------------------------------------------------------------
// Tracks a 3-point model
// implementing the POSIT algorithm for coplanar points as presented in
// [Denis Oberkampf, Daniel F. DeMenthon, Larry S. Davis: "Iterative Pose Estimation Using Coplanar Feature Points"]
class PointTracker final
{
public:
    PointTracker();
    // track the pose using the set of normalized point coordinates (x pos in range -0.5:0.5)
    // f : (focal length)/(sensor width)
    // dt : time since last call
    void track(const std::vector<vec2>& projected_points, const PointModel& model, f focal_length, bool dynamic_pose, int init_phase_timeout, int w, int h);
    Affine pose() { return X_CM; }
    vec2 project(const vec3& v_M, f focal_length);

private:
    // the points in model order
    struct PointOrder
    {
        vec2 points[PointModel::N_POINTS];
        PointOrder()
        {
            for (unsigned i = 0; i < PointModel::N_POINTS; i++)
                points[i] = vec2(0, 0);
        }
    };

    PointOrder find_correspondences(const std::vector<vec2>& projected_points, const PointModel &model);
    PointOrder find_correspondences_previous(const std::vector<vec2>& points, const PointModel &model, f focal_length, int w, int h);
    int POSIT(const PointModel& point_model, const PointOrder& order, f focal_length);  // The POSIT algorithm, returns the number of iterations

    Affine X_CM; // trafo from model to camera

    Timer t;
    bool init_phase;
};

#endif //POINTTRACKER_H
