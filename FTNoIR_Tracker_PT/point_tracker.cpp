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

using namespace cv;
using namespace boost;
using namespace std;

// ----------------------------------------------------------------------------
PointModel::PointModel(Vec3f M01, Vec3f M02)
	: M01(M01), 
	  M02(M02)
{	
	// calculate u
	u = M01.cross(M02);
	u /= norm(u);

	// calculate projection matrix on M01,M02 plane
	float s11 = M01.dot(M01);
	float s12 = M01.dot(M02);
	float s22 = M02.dot(M02);
	P = 1.0/(s11*s22-s12*s12) * Matx22f(s22, -s12, 
		                               -s12,  s11);

	// calculate d and d_order for simple freetrack-like point correspondence
	vector<Vec2f> points;
	points.push_back(Vec2f(0,0));
	points.push_back(Vec2f(M01[0], M01[1]));
	points.push_back(Vec2f(M02[0], M02[1]));
	// fit line to orthographically projected points
	// ERROR: yields wrong results with colinear points?!
	/*
	Vec4f line;
	fitLine(points, line, CV_DIST_L2, 0, 0.01, 0.01);
	d[0] = line[0]; d[1] = line[1];
	*/
	// TODO: fix this
	d = Vec2f(M01[0]-M02[0], M01[1]-M02[1]);

	// sort model points
	get_d_order(points, d_order);
}

#ifdef OPENTRACK_API
static bool d_vals_sort(const pair<float,int> a, const pair<float,int> b)
{
    return a.first < b.first;
}
#endif

void PointModel::get_d_order(const std::vector<cv::Vec2f>& points, int d_order[]) const
{
	// get sort indices with respect to d scalar product
	vector< pair<float,int> > d_vals;
	for (int i = 0; i<points.size(); ++i)
		d_vals.push_back(pair<float, int>(d.dot(points[i]), i));

	struct
    {
        bool operator()(const pair<float, int>& a, const pair<float, int>& b) { return a.first < b.first; }
    } comp;
    std::sort(d_vals.begin(),
              d_vals.end(),
#ifdef OPENTRACK_API
              d_vals_sort
#else
              comp
#endif
              );

	for (int i = 0; i<points.size(); ++i)
		d_order[i] = d_vals[i].second;
}


// ----------------------------------------------------------------------------
PointTracker::PointTracker() 
	: init_phase(true),
	  dt_valid(0), 
	  dt_reset(1), 
	  v_t(0,0,0),
	  v_r(0,0,0), 
      dynamic_pose_resolution(true),
      fov(0),
      _w(0),
      _h(0)
{
}

void PointTracker::reset()
{
	// enter init phase and reset velocities
	init_phase = true;
	dt_valid = 0;
	reset_velocities();
    // assume identity rotation again
    X_CM.R = cv::Matx33f::eye();
    X_CM.t = cv::Vec3f(0, 0, 1);
}

void PointTracker::reset_velocities()
{	
	v_t = Vec3f(0,0,0);
	v_r = Vec3f(0,0,0);
}


bool PointTracker::track(const vector<Vec2f>& points, float fov, float dt, int w, int h)
{
	if (!dynamic_pose_resolution) init_phase = true;

	dt_valid += dt;
	// if there was no valid tracking result for too long, do a reset
	if (dt_valid > dt_reset) 
	{
		//qDebug()<<"dt_valid "<<dt_valid<<" > dt_reset "<<dt_reset;
		reset();
	}

    bool no_model = !point_model;

	// if there is a pointtracking problem, reset the velocities
    if (no_model || points.size() != PointModel::N_POINTS)
	{
		//qDebug()<<"Wrong number of points!";
		reset_velocities();
		return false;
	}

	X_CM_old = X_CM;	// backup old transformation for velocity calculation

	if (!init_phase) 
		predict(dt_valid);

	// if there is a point correspondence problem something has gone wrong, do a reset
    if (!find_correspondences(points))
	{
		//qDebug()<<"Error in finding point correspondences!";
		X_CM = X_CM_old;	// undo prediction
		reset();
		return false;
	}

    POSIT(fov, w, h);
	//qDebug()<<"Number of POSIT iterations: "<<n_iter;

	if (!init_phase)
		update_velocities(dt_valid);

	// we have a valid tracking result, leave init phase and reset time since valid result
	init_phase = false;
	dt_valid = 0;
	return true;
}

void PointTracker::predict(float dt)
{
	// predict with constant velocity
    Matx33f R;
	Rodrigues(dt*v_r, R);
	X_CM.R = R*X_CM.R;
	X_CM.t += dt * v_t;
}

void PointTracker::update_velocities(float dt)
{
	// update velocities
	Rodrigues(X_CM.R*X_CM_old.R.t(), v_r);
	v_r /= dt;
	v_t = (X_CM.t - X_CM_old.t)/dt;
}

bool PointTracker::find_correspondences(const vector<Vec2f>& points)
{
	if (init_phase) {
		// We do a simple freetrack-like sorting in the init phase...
		// sort points
		int point_d_order[PointModel::N_POINTS];
		point_model->get_d_order(points, point_d_order);

		// set correspondences
		for (int i=0; i<PointModel::N_POINTS; ++i)
		{
			p[point_model->d_order[i]] = points[point_d_order[i]];
		}
	}
	else {
		// ... otherwise we look at the distance to the projection of the expected model points 
		// project model points under current pose
        p_exp[0] = project(Vec3f(0,0,0));
        p_exp[1] = project(point_model->M01);
        p_exp[2] = project(point_model->M02);

		// set correspondences by minimum distance to projected model point
		bool point_taken[PointModel::N_POINTS];
		for (int i=0; i<PointModel::N_POINTS; ++i)
			point_taken[i] = false;

		float min_sdist = 0;
		int min_idx = 0;
		
		for (int i=0; i<PointModel::N_POINTS; ++i)
		{
			// find closest point to projected model point i
			for (int j=0; j<PointModel::N_POINTS; ++j)
			{
				Vec2f d = p_exp[i]-points[j];
				float sdist = d.dot(d);
				if (sdist < min_sdist || j==0) 
				{
					min_idx = j;
					min_sdist = sdist;
				}
			}
			// if one point is closest to more than one model point, abort
			if (point_taken[min_idx]) return false;
			point_taken[min_idx] = true;
			p[i] = points[min_idx];
		}
	}
	return true;
}



void PointTracker::POSIT(float fov, int w, int h)
{
    // XXX hack
    this->fov = fov;
    _w = w;
    _h = h;
    std::vector<cv::Point3f> obj_points;
    std::vector<cv::Point2f> img_points;

    obj_points.push_back(cv::Vec3f(0, 0, 0));
    obj_points.push_back(point_model->M01);
    obj_points.push_back(point_model->M02);

    img_points.push_back(p[0]);
    img_points.push_back(p[1]);
    img_points.push_back(p[2]);

    const float HT_PI = 3.1415926535;

    const float focal_length_w = 0.5 * w / tan(fov * HT_PI / 180);
    const float focal_length_h = 0.5 * h / tan(fov * h / w * HT_PI / 180.0);

    cv::Mat intrinsics = cv::Mat::eye(3, 3, CV_32FC1);
    intrinsics.at<float> (0, 0) = focal_length_w;
    intrinsics.at<float> (1, 1) = focal_length_h;
    intrinsics.at<float> (0, 2) = w/2;
    intrinsics.at<float> (1, 2) = h/2;

    cv::Mat dist_coeffs = cv::Mat::zeros(5, 1, CV_32FC1);

    bool lastp = !rvec.empty() && !tvec.empty();

    cv::solvePnP(obj_points, img_points, intrinsics, dist_coeffs, rvec, tvec, lastp, cv::ITERATIVE);

    cv::Mat rmat;
    cv::Rodrigues(rvec, rmat);

    // finally, find the closer solution
    cv::Mat expected(3, 3, CV_64FC1);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            expected.at<double>(i, j) = X_CM.R(i, j);
    cv::Mat eye = cv::Mat::eye(3, 3, CV_64FC1);
    double dev1 = norm(eye - expected * rmat.t());
    double dev2 = norm(eye - expected * rmat);

    if (dev1 > dev2)
    {
        rmat = rmat.t();
        cv::Rodrigues(rmat, rvec);
    }

	// apply results
    for (int i = 0; i < 3; i++)
    {
        X_CM.t[i] = tvec.at<double>(i);
        for (int j = 0; j < 3; j++)
            X_CM.R(i, j) = rmat.at<double>(i, j);
    }
}
