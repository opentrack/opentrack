/* Copyright (c) 2019, Stephane Lenclud <github@lenclud.com>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "kalman-filter-pose.h"

namespace EasyTracker
{

    KalmanFilterPose::KalmanFilterPose()
    {

    }


    void KalmanFilterPose::Init(int nStates, int nMeasurements, int nInputs, double dt)
    {
        iMeasurements = cv::Mat(nMeasurements, 1, CV_64FC1);

        init(nStates, nMeasurements, nInputs, CV_64F);                 // init Kalman Filter

        // TODO: Use parameters instead of magic numbers
        setIdentity(processNoiseCov, cv::Scalar::all(1)); //1e-5      // set process noise
        setIdentity(measurementNoiseCov, cv::Scalar::all(1)); //1e-2   // set measurement noise
        setIdentity(errorCovPost, cv::Scalar::all(1));             // error covariance

        /** DYNAMIC MODEL **/

        //  [1 0 0 dt  0  0 dt2   0   0 0 0 0  0  0  0   0   0   0]
        //  [0 1 0  0 dt  0   0 dt2   0 0 0 0  0  0  0   0   0   0]
        //  [0 0 1  0  0 dt   0   0 dt2 0 0 0  0  0  0   0   0   0]
        //  [0 0 0  1  0  0  dt   0   0 0 0 0  0  0  0   0   0   0]
        //  [0 0 0  0  1  0   0  dt   0 0 0 0  0  0  0   0   0   0]
        //  [0 0 0  0  0  1   0   0  dt 0 0 0  0  0  0   0   0   0]
        //  [0 0 0  0  0  0   1   0   0 0 0 0  0  0  0   0   0   0]
        //  [0 0 0  0  0  0   0   1   0 0 0 0  0  0  0   0   0   0]
        //  [0 0 0  0  0  0   0   0   1 0 0 0  0  0  0   0   0   0]
        //  [0 0 0  0  0  0   0   0   0 1 0 0 dt  0  0 dt2   0   0]
        //  [0 0 0  0  0  0   0   0   0 0 1 0  0 dt  0   0 dt2   0]
        //  [0 0 0  0  0  0   0   0   0 0 0 1  0  0 dt   0   0 dt2]
        //  [0 0 0  0  0  0   0   0   0 0 0 0  1  0  0  dt   0   0]
        //  [0 0 0  0  0  0   0   0   0 0 0 0  0  1  0   0  dt   0]
        //  [0 0 0  0  0  0   0   0   0 0 0 0  0  0  1   0   0  dt]
        //  [0 0 0  0  0  0   0   0   0 0 0 0  0  0  0   1   0   0]
        //  [0 0 0  0  0  0   0   0   0 0 0 0  0  0  0   0   1   0]
        //  [0 0 0  0  0  0   0   0   0 0 0 0  0  0  0   0   0   1]

        // position
        transitionMatrix.at<double>(0, 3) = dt;
        transitionMatrix.at<double>(1, 4) = dt;
        transitionMatrix.at<double>(2, 5) = dt;
        transitionMatrix.at<double>(3, 6) = dt;
        transitionMatrix.at<double>(4, 7) = dt;
        transitionMatrix.at<double>(5, 8) = dt;
        transitionMatrix.at<double>(0, 6) = 0.5*pow(dt, 2);
        transitionMatrix.at<double>(1, 7) = 0.5*pow(dt, 2);
        transitionMatrix.at<double>(2, 8) = 0.5*pow(dt, 2);

        // orientation
        transitionMatrix.at<double>(9, 12) = dt;
        transitionMatrix.at<double>(10, 13) = dt;
        transitionMatrix.at<double>(11, 14) = dt;
        transitionMatrix.at<double>(12, 15) = dt;
        transitionMatrix.at<double>(13, 16) = dt;
        transitionMatrix.at<double>(14, 17) = dt;
        transitionMatrix.at<double>(9, 15) = 0.5*pow(dt, 2);
        transitionMatrix.at<double>(10, 16) = 0.5*pow(dt, 2);
        transitionMatrix.at<double>(11, 17) = 0.5*pow(dt, 2);


        /** MEASUREMENT MODEL **/

        //  [1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]
        //  [0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]
        //  [0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0]
        //  [0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0]
        //  [0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0]
        //  [0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0]

        // Those will scale the filtered values, 2 will give you half the raw input
        measurementMatrix.at<double>(0, 0) = 1;  // x
        measurementMatrix.at<double>(1, 1) = 1;  // y
        measurementMatrix.at<double>(2, 2) = 1;  // z
        measurementMatrix.at<double>(3, 9) = 1;  // roll
        measurementMatrix.at<double>(4, 10) = 1; // pitch
        measurementMatrix.at<double>(5, 11) = 1; // yaw
    }

    void KalmanFilterPose::Update(double& aX, double& aY, double& aZ, double& aRoll, double& aPitch, double& aYaw)
    {
        // Set measurement to predict
        iMeasurements.at<double>(0) = aX; // x
        iMeasurements.at<double>(1) = aY; // y
        iMeasurements.at<double>(2) = aZ; // z
        iMeasurements.at<double>(3) = aRoll;      // roll
        iMeasurements.at<double>(4) = aPitch;      // pitch
        iMeasurements.at<double>(5) = aYaw;      // yaw

        // First predict, to update the internal statePre variable
        cv::Mat prediction = predict();
        // The "correct" phase that is going to use the predicted value and our measurement
        cv::Mat estimated = correct(iMeasurements);
        // Estimated translation
        aX = estimated.at<double>(0);
        aY = estimated.at<double>(1);
        aZ = estimated.at<double>(2);
        // Estimated euler angles        
        aRoll = estimated.at<double>(9);
        aPitch = estimated.at<double>(10);
        aYaw = estimated.at<double>(11);
    }

}

