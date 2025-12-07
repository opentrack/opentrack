#include "model_adapters.h"

#include <algorithm>
#include <cstdio>
#include <numeric>

namespace neuralnet_tracker_tests
{

void assert_(bool ok, const std::string& msg)
{
    if (ok)
        return;
    std::cout << msg << std::endl;
    std::exit(-1);
}

void test_find_input_intensity_quantile()
{
    cv::Mat data(10, 10, CV_8UC1);
    std::iota(data.begin<uint8_t>(), data.end<uint8_t>(), 0);

    const float pct = 90;

    const int val = neuralnet_tracker_ns::find_input_intensity_quantile(data, pct);

    assert_(val == int(10 * 10 * pct / 100.f), "test_find_input_intensity_quantile failed");
}

void test_normalize_brightness()
{
    cv::Mat data(10, 10, CV_8UC1);
    std::iota(data.begin<uint8_t>(), data.end<uint8_t>(), 0);

    cv::Mat out;
    neuralnet_tracker_ns::normalize_brightness(data, out);

    auto [minit, maxit] = std::minmax_element(out.begin<float>(), out.end<float>());
    const auto minval = *minit;
    const auto maxval = *maxit;
    assert_(std::abs(minval + 0.5f) < 0.02, "test_normalize_brightness failed");
    // If the brightest value is lower than half-max, it will be boosted to half-max.
    // Otherwise it will just be rescaled to [-.5, 0.5 ]. Here we have the low-brightness case.
    assert_(std::abs(maxval - 0.0f) < 0.02, "test_normalize_brightness failed");
}

void run()
{
    test_find_input_intensity_quantile();
    test_normalize_brightness();
}

} // namespace neuralnet_tracker_tests