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

#ifdef OPENTRACK_API
static bool d_vals_sort(const pair<float,int> a, const pair<float,int> b)
{
    return a.first < b.first;
}
#endif

void PointModel::get_d_order(const std::vector<cv::Vec2f>& points, int d_order[], cv::Vec2f d) const
{
	// fit line to orthographically projected points
	vector< pair<float,int> > d_vals;
    // get sort indices with respect to d scalar product
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


PointTracker::PointTracker()
{
	X_CM.t[2] = 1000;	// default position: 1 m away from cam;
}

void PointTracker::track(const vector<Vec2f>& points, const PointModel& model, float f)
{
    const PointOrder& order = find_correspondences(points, model);
    POSIT(model, order, f);
}

PointTracker::PointOrder PointTracker::find_correspondences(const std::vector<cv::Vec2f>& points, const PointModel& model)
{
    // We do a simple freetrack-like sorting in the init phase...
    // sort points
    int point_d_order[PointModel::N_POINTS];
    int model_d_order[PointModel::N_POINTS];
    cv::Vec2f d(points[0][0]-points[1][0], points[0][1]-points[1][1]);
    model.get_d_order(points, point_d_order, d);
    // calculate d and d_order for simple freetrack-like point correspondence
    model.get_d_order(std::vector<cv::Vec2f> {
                          Vec2f{0,0},
                          Vec2f(model.M01[0], model.M01[1]),
                          Vec2f(model.M02[0], model.M02[1])
                      },
                      model_d_order,
                      d);
    // set correspondences
    PointOrder p;
    for (int i=0; i<PointModel::N_POINTS; ++i)
        p.points[model_d_order[i]] = points[point_d_order[i]];

    return p;
}

int PointTracker::POSIT(const PointModel& model, const PointOrder& order_, float focal_length)
{
	// POSIT algorithm for coplanar points as presented in
	// [Denis Oberkampf, Daniel F. DeMenthon, Larry S. Davis: "Iterative Pose Estimation Using Coplanar Feature Points"]
	// we use the same notation as in the paper here

	// The expected rotation used for resolving the ambiguity in POSIT:
	// In every iteration step the rotation closer to R_expected is taken 
	Matx33f R_expected = Matx33f::eye();	
	
	// initial pose = last (predicted) pose
	Vec3f k;
        get_row(R_expected, 2, k);
        float Z0 = 1000.f;

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
