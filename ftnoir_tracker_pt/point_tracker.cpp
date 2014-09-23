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
using namespace std;

const float PI = 3.14159265358979323846f;

// ----------------------------------------------------------------------------
static void get_row(const Matx33f& m, int i, Vec3f& v)
{
	v[0] = m(i,0);
	v[1] = m(i,1);
	v[2] = m(i,2);
}

static void set_row(Matx33f& m, int i, const Vec3f& v)
{
	m(i,0) = v[0];
	m(i,1) = v[1];
	m(i,2) = v[2];
}

PointModel::PointModel() :
    M01 { 0, 0, 0 },
    M02 { 0, 0, 0 }
{
}

PointModel::PointModel(Vec3f M01, Vec3f M02)
	: M01(M01), M02(M02)
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
	for (unsigned i = 0; i<points.size(); ++i)
		d_vals.push_back(pair<float, int>(d.dot(points[i]), i));

    std::sort(d_vals.begin(),
              d_vals.end(),
#ifdef OPENTRACK_API
              d_vals_sort
#else
              comp
#endif
              );

	for (unsigned i = 0; i<points.size(); ++i)
		d_order[i] = d_vals[i].second;
}


// ----------------------------------------------------------------------------
PointTracker::PointTracker()
{
	X_CM.t[2] = 1000;	// default position: 1 m away from cam;
}

void PointTracker::reset()
{
	// enter init phase
	X_CM = FrameTrafo();
}

void PointTracker::track(const vector<Vec2f>& projected_points, const PointModel& model)
{
    const PointOrder& order = find_correspondences(projected_points, model);
	int iters = POSIT(model, order);
	qDebug()<<"POSIT iterations:"<<iters;
}

PointTracker::PointOrder PointTracker::find_correspondences(const std::vector<cv::Vec2f>& projected_points, const PointModel& model)
{
    // ... otherwise we look at the distance to the projection of the expected model points 
    // project model points under current pose
    Vec2f p_exp[3];
    p_exp[0] = project(Vec3f(0,0,0));
    p_exp[1] = project(model.get_M01());
    p_exp[2] = project(model.get_M02());
    
    // set correspondences by minimum distance to projected model point
    bool point_taken[PointModel::N_POINTS];
    for (int i=0; i<PointModel::N_POINTS; ++i)
        point_taken[i] = false;
    
    PointOrder p;
    for (int i=0; i<PointModel::N_POINTS; ++i)
        p.points[i] = Vec2f(0, 0);
    
    for (int i=0; i<PointModel::N_POINTS; ++i)
    {
        float min_sdist = 1e4;
        int min_idx = 0;
        // find closest point to projected model point i
        for (int j=0; j<PointModel::N_POINTS; ++j)
        {
            Vec2f d = p_exp[i]-projected_points[j];
            float sdist = d.dot(d);
            if (sdist < min_sdist)
            {
                min_idx = j;
                min_sdist = sdist;
            }
        }
        // if one point is closest to more than one model point, abort
        if (point_taken[min_idx]) return p;
        point_taken[min_idx] = true;
        p.points[i] = projected_points[min_idx];
    }
    return p;
}



int PointTracker::POSIT(const PointModel& model, const PointOrder& order_)
{
	// POSIT algorithm for coplanar points as presented in
	// [Denis Oberkampf, Daniel F. DeMenthon, Larry S. Davis: "Iterative Pose Estimation Using Coplanar Feature Points"]
	// we use the same notation as in the paper here

	// The expected rotation used for resolving the ambiguity in POSIT:
	// In every iteration step the rotation closer to R_expected is taken 
	Matx33f R_expected;	
    R_expected = X_CM.R; // later we want to be close to the last (predicted) rotation
	
	// initial pose = last (predicted) pose
	Vec3f k;
    get_row(R_expected, 2, k);
    float Z0 = std::abs(X_CM.t[2]) < 1e-3 ? 1e3 : X_CM.t[2];

	float old_epsilon_1 = 0;
	float old_epsilon_2 = 0;
	float epsilon_1 = 1;
	float epsilon_2 = 1;

	Vec3f I0, J0;
	Vec2f I0_coeff, J0_coeff;

	Vec3f I_1, J_1, I_2, J_2;
	Matx33f R_1, R_2;
	Matx33f* R_current;

	const int MAX_ITER = 100;
	const float EPS_THRESHOLD = 1e-4;
    
    const cv::Vec2f* order = order_.points;

	int i=1;
	for (; i<MAX_ITER; ++i)
	{
		epsilon_1 = k.dot(model.M01)/Z0;
		epsilon_2 = k.dot(model.M02)/Z0;

		// vector of scalar products <I0, M0i> and <J0, M0i>
		Vec2f I0_M0i(order[1][0]*(1.0 + epsilon_1) - order[0][0], 
			         order[2][0]*(1.0 + epsilon_2) - order[0][0]);
		Vec2f J0_M0i(order[1][1]*(1.0 + epsilon_1) - order[0][1],
			         order[2][1]*(1.0 + epsilon_2) - order[0][1]);

		// construct projection of I, J onto M0i plane: I0 and J0
		I0_coeff = model.P * I0_M0i;
		J0_coeff = model.P * J0_M0i;
		I0 = I0_coeff[0]*model.M01 + I0_coeff[1]*model.M02;
		J0 = J0_coeff[0]*model.M01 + J0_coeff[1]*model.M02;

		// calculate u component of I, J		
		float II0 = I0.dot(I0);
		float IJ0 = I0.dot(J0);
		float JJ0 = J0.dot(J0);
		float rho, theta;
		if (JJ0 == II0) {
			rho = sqrt(abs(2*IJ0));
			theta = -PI/4;
			if (IJ0<0) theta *= -1;
		}
		else {
			rho = sqrt(sqrt( (JJ0-II0)*(JJ0-II0) + 4*IJ0*IJ0 ));
			theta = atan( -2*IJ0 / (JJ0-II0) );
			if (JJ0 - II0 < 0) theta += PI;
			theta /= 2;
		}

		// construct the two solutions
		I_1 = I0 + rho*cos(theta)*model.u;	
		I_2 = I0 - rho*cos(theta)*model.u;

		J_1 = J0 + rho*sin(theta)*model.u;
		J_2 = J0 - rho*sin(theta)*model.u;

		float norm_const = 1.0/norm(I_1); // all have the same norm
		
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
		float R_1_deviation = norm(Matx33f::eye() - R_expected * R_1.t());
		float R_2_deviation = norm(Matx33f::eye() - R_expected * R_2.t());

		if (R_1_deviation < R_2_deviation)
			R_current = &R_1;
		else
			R_current = &R_2;

		get_row(*R_current, 2, k);

		// check for convergence condition
		if (abs(epsilon_1 - old_epsilon_1) +  abs(epsilon_2 - old_epsilon_2) < EPS_THRESHOLD)
			break;
		old_epsilon_1 = epsilon_1;
		old_epsilon_2 = epsilon_2;
	}	

	// apply results
	X_CM.R = *R_current;
	X_CM.t[0] = order[0][0] * Z0/focal_length;
	X_CM.t[1] = order[0][1] * Z0/focal_length;
	X_CM.t[2] = Z0;

	return i;

	//Rodrigues(X_CM.R, r);
	//qDebug()<<"iter: "<<i;
	//qDebug()<<"t: "<<X_CM.t[0]<<' '<<X_CM.t[1]<<' '<<X_CM.t[2];
	//Vec3f r;
	//
	//qDebug()<<"r: "<<r[0]<<' '<<r[1]<<' '<<r[2]<<'\n';
}
