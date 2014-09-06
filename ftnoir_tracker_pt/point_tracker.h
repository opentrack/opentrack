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
	bool track(const std::vector<cv::Vec2f>& points, float f, float dt);
	boost::shared_ptr<PointModel> point_model;

	bool dynamic_pose_resolution;
	float dt_reset;

	FrameTrafo get_pose() const { return X_CM; }
	void reset();

protected:
	inline cv::Vec2f project(const cv::Vec3f& v_M, float f)
	{
		cv::Vec3f v_C = X_CM * v_M;
		return cv::Vec2f(f*v_C[0]/v_C[2], f*v_C[1]/v_C[2]);
	}

	bool find_correspondences(const std::vector<cv::Vec2f>& points, float f);

	cv::Vec2f p[PointModel::N_POINTS];	// the points in model order
	cv::Vec2f p_exp[PointModel::N_POINTS];	// the expected point positions

	void predict(float dt);
	void update_velocities(float dt);
	void reset_velocities();

	
	int POSIT(float f);  // The POSIT algorithm, returns the number of iterations

	bool init_phase;
	float dt_valid;	// time since last valid tracking result
	cv::Vec3f v_t;	// velocities
	cv::Vec3f v_r;
	FrameTrafo X_CM; // trafo from model to camera
	FrameTrafo X_CM_old;
};

#endif //POINTTRACKER_H
