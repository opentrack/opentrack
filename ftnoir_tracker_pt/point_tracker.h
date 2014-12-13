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

    PointModel(cv::Vec3f M01, cv::Vec3f M02);
    PointModel();

    inline const cv::Vec3f& get_M01() const { return M01; }
    inline const cv::Vec3f& get_M02() const { return M02; }

private:
    cv::Vec3f M01;	// M01 in model frame
    cv::Vec3f M02;	// M02 in model frame

    cv::Vec3f u;	// unit vector perpendicular to M01,M02-plane

    cv::Matx22f P;
    
    template<typename vec>
    void get_d_order(const std::vector<vec>& points, int* d_order, vec d) const;
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
    void track(const std::vector<cv::Vec2f>& projected_points, const PointModel& model, float f);
    Affine pose() const { return X_CM; }
    
private:
    // the points in model order
    typedef struct { cv::Vec2f points[PointModel::N_POINTS]; } PointOrder;

    PointOrder find_correspondences(const std::vector<cv::Vec2f>& projected_points, const PointModel &model);
    int POSIT(const PointModel& point_model, const PointOrder& order, float focal_length);  // The POSIT algorithm, returns the number of iterations

    Affine X_CM; // trafo from model to camera
};

#endif //POINTTRACKER_H
