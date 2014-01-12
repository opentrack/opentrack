/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef POINTTRACKER_H
#define POINTTRACKER_H

#include <opencv2/opencv.hpp>
#ifndef OPENTRACK_API
#   include <boost/shared_ptr.hpp>
#else
#   include "FTNoIR_Tracker_PT/boost-compat.h"
#endif
#include <list>

// ----------------------------------------------------------------------------
// Affine frame trafo
class FrameTrafo
{
public:
    FrameTrafo() : R(cv::Matx33f::eye()), t(0,0,0) {}
    FrameTrafo(const cv::Matx33f& R, const cv::Vec3f& t) : R(R),t(t) {}

    cv::Matx33f R;
    cv::Vec3f t;
};

inline FrameTrafo operator*(const FrameTrafo& X, const FrameTrafo& Y)
{
	return FrameTrafo(X.R*Y.R, X.R*Y.t + X.t);
}

inline FrameTrafo operator*(const cv::Matx33f& X, const FrameTrafo& Y)
{
	return FrameTrafo(X*Y.R, X*Y.t);
}

inline FrameTrafo operator*(const FrameTrafo& X, const cv::Matx33f& Y)
{
	return FrameTrafo(X.R*Y, X.t);
}

inline cv::Vec3f operator*(const FrameTrafo& X, const cv::Vec3f& v)
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
	static const int N_POINTS = 3;

	PointModel(cv::Vec3f M01, cv::Vec3f M02);

	const cv::Vec3f& get_M01() const { return M01; };
	const cv::Vec3f& get_M02() const { return M02; };

protected:
	cv::Vec3f M01;	// M01 in model frame
	cv::Vec3f M02;	// M02 in model frame

	cv::Vec3f u;	// unit vector perpendicular to M01,M02-plane

	cv::Matx22f P;

	cv::Vec2f d;	// discriminant vector for point correspondence
	int d_order[3];	// sorting of projected model points with respect to d scalar product 

	void get_d_order(const std::vector<cv::Vec2f>& points, int d_order[]) const;
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
    bool track(const std::vector<cv::Vec2f>& points, float fov, float dt, int w, int h);
	boost::shared_ptr<PointModel> point_model;

	bool dynamic_pose_resolution;
	float dt_reset;

	FrameTrafo get_pose() const { return X_CM; }
	void reset();
protected:
    cv::Vec2f project(const cv::Vec3f& v_M, float fov, int w, int h)
	{
        if (!rvec.empty() && !tvec.empty() && fov > 0)
        {
            const float HT_PI = 3.1415926535;
            const float focal_length_w = 0.5 * w / tan(fov * HT_PI / 180);
            const float focal_length_h = 0.5 * h / tan(fov * h / w * HT_PI / 180.0);

            cv::Mat intrinsics = cv::Mat::eye(3, 3, CV_32FC1);
            intrinsics.at<float> (0, 0) = focal_length_w;
            intrinsics.at<float> (1, 1) = focal_length_h;
            intrinsics.at<float> (0, 2) = w/2;
            intrinsics.at<float> (1, 2) = h/2;
            std::vector<cv::Point3f> xs;
            xs.push_back(v_M);
            cv::Mat dist_coeffs = cv::Mat::zeros(5, 1, CV_32FC1);
            std::vector<cv::Point2f> rets(1);
            cv::projectPoints(xs, rvec, tvec, intrinsics, dist_coeffs, rets);
            return rets[0];
        }
        return cv::Vec2f();
	}

    bool find_correspondences(const std::vector<cv::Vec2f>& points, float fov, int w, int h);

	cv::Vec2f p[PointModel::N_POINTS];	// the points in model order
	cv::Vec2f p_exp[PointModel::N_POINTS];	// the expected point positions

	void predict(float dt);
	void update_velocities(float dt);
	void reset_velocities();

	
    void POSIT(float fov, int w, int h);  // The POSIT algorithm, returns the number of iterations

	bool init_phase;
	float dt_valid;	// time since last valid tracking result
	cv::Vec3f v_t;	// velocities
	cv::Vec3f v_r;
	FrameTrafo X_CM; // trafo from model to camera
	FrameTrafo X_CM_old;
    cv::Mat rvec, tvec;
};

#endif //POINTTRACKER_H
