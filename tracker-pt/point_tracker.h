/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef POINTTRACKER_H
#define POINTTRACKER_H

#include <opencv2/core/core.hpp>
#include <memory>
#include <vector>
#include "opentrack-compat/timer.hpp"
#include "ftnoir_tracker_pt_settings.h"

#include <QObject>

class Affine
{
public:
    Affine() : R(cv::Matx33d::eye()), t(0,0,0) {}
    Affine(const cv::Matx33d& R, const cv::Vec3d& t) : R(R),t(t) {}

    cv::Matx33d R;
    cv::Vec3d t;
};

inline Affine operator*(const Affine& X, const Affine& Y)
{
    return Affine(X.R*Y.R, X.R*Y.t + X.t);
}

inline Affine operator*(const cv::Matx33d& X, const Affine& Y)
{
    return Affine(X*Y.R, X*Y.t);
}

inline Affine operator*(const Affine& X, const cv::Matx33d& Y)
{
    return Affine(X.R*Y, X.t);
}

inline cv::Vec3d operator*(const Affine& X, const cv::Vec3d& v)
{
    return X.R*v + X.t;
}


// ----------------------------------------------------------------------------
// Describes a 3-point model
// nomenclature as in
// [Denis Oberkampf, Daniel F. DeMenthon, Larry S. Davis: "Iterative Pose Estimation Using Coplanar Feature Points"]
class PointModel
{
    friend class PointTracker;
public:
    static constexpr int N_POINTS = 3;

    cv::Vec3d M01;      // M01 in model frame
    cv::Vec3d M02;      // M02 in model frame

    cv::Vec3d u;        // unit vector perpendicular to M01,M02-plane

    cv::Matx22d P;

    PointModel(settings_pt& s)
    {
        set_model(s);
        // calculate u
        u = M01.cross(M02);
        u /= norm(u);

        // calculate projection matrix on M01,M02 plane
        double s11 = M01.dot(M01);
        double s12 = M01.dot(M02);
        double s22 = M02.dot(M02);
        P = 1.0/(s11*s22-s12*s12) * cv::Matx22d(s22, -s12, -s12,  s11);
    }

    void set_model(settings_pt& s)
    {
        enum { Cap = 0, ClipRight = 1, ClipLeft = 2 };

        switch (s.model_used)
        {
        default:
        case Cap:
        {
            const double x = 60, y = 90, z = 95;
            M01 = cv::Vec3d(-x, -y, z);
            M02 = cv::Vec3d(x, -y, z);
            break;
        }
        case ClipLeft:
        case ClipRight:
        {
            const double a = 27, b = 43, c = 62, d = 74;
            M01 = cv::Vec3d(0, b, -a);
            M02 = cv::Vec3d(0, -c, -d);
            break;
        }
        }
    }

    void get_d_order(const std::vector<cv::Vec2d>& points, int* d_order, const cv::Vec2d& d) const;
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
    void track(const std::vector<cv::Vec2d>& projected_points, const PointModel& model, double focal_length, bool dynamic_pose, int init_phase_timeout, int w, int h);
    Affine pose() { return X_CM; }
    cv::Vec2d project(const cv::Vec3d& v_M, double focal_length);
private:
    // the points in model order
    struct PointOrder
    {
        cv::Vec2d points[PointModel::N_POINTS];
        PointOrder()
        {
            for (int i = 0; i < PointModel::N_POINTS; i++)
                points[i] = cv::Vec2d(0, 0);
        }
    };

    PointOrder find_correspondences(const std::vector<cv::Vec2d>& projected_points, const PointModel &model);
    PointOrder find_correspondences_previous(const std::vector<cv::Vec2d>& points, const PointModel &model, double focal_length, int w, int h);
    bool POSIT(const PointModel& point_model, const PointOrder& order, double focal_length);  // The POSIT algorithm, returns the number of iterations

    Affine X_CM; // trafo from model to camera

    Timer t;
    bool init_phase;
};

#endif //POINTTRACKER_H
