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
#include "ftnoir_tracker_wiimote_settings.h"

#include <QObject>
#include <QMutex>

class Affine
{
public:
    Affine() : R(cv::Matx33f::eye()), t(0,0,0) {}
    Affine(const cv::Matx33f& R, const cv::Vec3f& t) : R(R),t(t) {}

    cv::Matx33f R;
    cv::Vec3f t;
};

inline Affine operator*(const Affine& X, const Affine& Y)
{
    return Affine(X.R*Y.R, X.R*Y.t + X.t);
}

inline Affine operator*(const cv::Matx33f& X, const Affine& Y)
{
    return Affine(X*Y.R, X*Y.t);
}

inline Affine operator*(const Affine& X, const cv::Matx33f& Y)
{
    return Affine(X.R*Y, X.t);
}

inline cv::Vec3f operator*(const Affine& X, const cv::Vec3f& v)
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

    cv::Vec3f M01;      // M01 in model frame
    cv::Vec3f M02;      // M02 in model frame

    cv::Vec3f u;        // unit vector perpendicular to M01,M02-plane

    cv::Matx22f P;
    
    enum Model { Clip = 0, Cap = 1, Custom = 2 };

    PointModel(settings_wiimote& s)
    {
        set_model(s);
        // calculate u
        u = M01.cross(M02);
        u /= norm(u);

        // calculate projection matrix on M01,M02 plane
        float s11 = M01.dot(M01);
        float s12 = M01.dot(M02);
        float s22 = M02.dot(M02);
        P = 1.0/(s11*s22-s12*s12) * cv::Matx22f(s22, -s12, -s12,  s11);
    }
    
    void set_model(settings_wiimote& s)
    {
        switch (s.active_model_panel)
        {
        case Clip:
            M01 = cv::Vec3f(0, static_cast<double>(s.clip_ty), -static_cast<double>(s.clip_tz));
            M02 = cv::Vec3f(0, -static_cast<double>(s.clip_by), -static_cast<double>(s.clip_bz));
            break;
        case Cap:
            M01 = cv::Vec3f(-static_cast<double>(s.cap_x), -static_cast<double>(s.cap_y), -static_cast<double>(s.cap_z));
            M02 = cv::Vec3f(static_cast<double>(s.cap_x), -static_cast<double>(s.cap_y), -static_cast<double>(s.cap_z));
            break;
        case Custom:
            M01 = cv::Vec3f(s.m01_x, s.m01_y, s.m01_z);
            M02 = cv::Vec3f(s.m02_x, s.m02_y, s.m02_z);
            break;
        }
    }
    
    void get_d_order(const std::vector<cv::Vec2f>& points, int* d_order, cv::Vec2f d) const;
};

// ----------------------------------------------------------------------------
// Tracks a 3-point model
// implementing the POSIT algorithm for coplanar points as presented in
// [Denis Oberkampf, Daniel F. DeMenthon, Larry S. Davis: "Iterative Pose Estimation Using Coplanar Feature Points"]
class PointTracker
{
public:
    PointTracker();
    // track the pose using the set of normalized point coordinates (x pos in range -0.5:0.5)
    // f : (focal length)/(sensor width)
    // dt : time since last call
    void track(const std::vector<cv::Vec2f>& projected_points, const PointModel& model, float f, bool dynamic_pose, int init_phase_timeout);
    Affine pose() { QMutexLocker l(&mtx); return X_CM; }
    cv::Vec2f project(const cv::Vec3f& v_M, float f);
private:
    // the points in model order
    struct PointOrder
    {
        cv::Vec2f points[PointModel::N_POINTS];
        PointOrder()
        {
            for (int i = 0; i < PointModel::N_POINTS; i++)
                points[i] = cv::Vec2f(0, 0);
        }
    };

    PointOrder find_correspondences(const std::vector<cv::Vec2f>& projected_points, const PointModel &model);
    PointOrder find_correspondences_previous(const std::vector<cv::Vec2f>& points, const PointModel &model, float f);
    int POSIT(const PointModel& point_model, const PointOrder& order, float focal_length);  // The POSIT algorithm, returns the number of iterations

    Affine X_CM; // trafo from model to camera

    Timer t;
    bool init_phase;
    QMutex mtx;
};

#endif //POINTTRACKER_H
