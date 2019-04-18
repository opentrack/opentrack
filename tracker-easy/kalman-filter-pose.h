/* Copyright (c) 2019, Stephane Lenclud <github@lenclud.com>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <opencv2/core.hpp>
#include <opencv2/video/tracking.hpp>


namespace EasyTracker
{

    ///
    /// TODO: do not use a constant time difference
    ///
    class KalmanFilterPose: public cv::KalmanFilter
    {
    public:
        KalmanFilterPose();
        void Init(int aStateCount, int aMeasurementCount, int aInputCount, double aDt);
        void Update(double& aX, double& aY, double& aZ, double& aRoll, double& aPitch, double& aYaw);


        cv::Mat iMeasurements;
    };

}
