/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef POINTTRACKER_H
#define POINTTRACKER_H

#include <opencv2/core/core.hpp>
#ifndef OPENTRACK_API
#   include <boost/shared_ptr.hpp>
#else
#   include <memory>
#endif
#include <vector>

#include "ftnoir_tracker_pt_settings.h"

#include <QObject>

// ----------------------------------------------------------------------------
// Affine frame trafo
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

    cv::Vec3f M01;	// M01 in model frame
    cv::Vec3f M02;	// M02 in model frame

    cv::Vec3f u;	// unit vector perpendicular to M01,M02-plane

    cv::Matx22f P;
    
    PointModel(settings_pt& s)
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
    
    enum { Cap = 0, Clip = 1 };
    
    void set_model(settings_pt& s)
    {
        if (s.model_used == Cap)
        {
            const double z = 100, x = 120, y = 60;
            M01 = cv::Vec3f(-x, -y, -z);
            M02 = cv::Vec3f(x, -y, -z);
        }
        else
        {
            const double a = 27, b = 43, c = 62, d = 74;
            M01 = cv::Vec3f(0, b, -a);
            M02 = cv::Vec3f(0, -c, -d);            
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
    void track(const std::vector<cv::Vec2f>& projected_points, const PointModel& model, float f, bool dynamic_pose);
    Affine pose() const { return X_CM; }
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
};

#endif //POINTTRACKER_H
