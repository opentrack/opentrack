#include <opencv2/calib3d.hpp>
void check_solvep3p()
{
    cv::Mat x;
    cv::solveP3P(x, x, x, x, x, x, 0);
}

#include <opencv2/video/tracking.hpp>
void check_kf()
{
    [[maybe_unused]] cv::KalmanFilter kf;
}

#include <opencv2/highgui.hpp>
void check_highgui()
{
    cv::imshow("foo", cv::noArray());
}
