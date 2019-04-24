#include <opencv2/calib3d.hpp>

static void check_solvep3p()
{
    cv::Mat x;
    cv::solveP3P(x, x, x, x, x, x, 0);
}
