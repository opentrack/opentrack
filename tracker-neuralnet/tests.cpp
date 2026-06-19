#include "model_adapters.h"
#include "ftnoir_tracker_neuralnet.h"
#include "compat/macros.h"

#include <algorithm>
#include <cstdio>
#include <numeric>

namespace neuralnet_tracker_tests
{

void assert_impl_(bool ok, [[maybe_unused]] const char* file, int line, const char* func, const char* condition_str)
{
    if (ok)
        return;
    std::cerr << "Failed tracker-neuralnet test, " << func << "L" << line << ", assert: " << condition_str << std::endl;
    std::exit(-1);
}

#define assert_(condition) assert_impl_(condition, __FILE__, __LINE__, tr_function_name, #condition)



void test_find_input_intensity_quantile()
{
    cv::Mat data(10, 10, CV_8UC1);
    std::iota(data.begin<uint8_t>(), data.end<uint8_t>(), 0);

    const float pct = 90;

    const int val = neuralnet_tracker_ns::find_input_intensity_quantile(data, pct);

    assert_(val == int(10 * 10 * pct / 100.f));
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
    assert_(std::abs(minval + 0.5f) < 0.02);
    // If the brightest value is lower than half-max, it will be boosted to half-max.
    // Otherwise it will just be rescaled to [-.5, 0.5 ]. Here we have the low-brightness case.
    assert_(std::abs(maxval - 0.0f) < 0.02);
}

using neuralnet_tracker_ns::ImagePyramid;
using neuralnet_tracker_ns::get_matching_level;

static constexpr int BIG_PYRAMID_SIZE = 7680;
static constexpr int SMALL_PYRAMID_SIZE = 320;

cv::Mat make_image(int width)
{
    return cv::Mat((width*10)/16, width, CV_8UC3, cv::Scalar(42));
}

void test_image_pyramid()
{
    ImagePyramid pyramid;
    pyramid.init(make_image(BIG_PYRAMID_SIZE));
    {
        
        assert_(pyramid.max_levels == 4);
        assert_(pyramid.depth() == 4);
        assert_(pyramid.image().cols == BIG_PYRAMID_SIZE);
        assert_(pyramid.image(1).cols == BIG_PYRAMID_SIZE/2);
        assert_(pyramid.image(2).cols == BIG_PYRAMID_SIZE/4);
        assert_(pyramid.image(3).cols == BIG_PYRAMID_SIZE/8);
        assert_(pyramid.coarsest_image().cols == BIG_PYRAMID_SIZE/8);
        assert_(pyramid.coarsest_image().type() == CV_8UC3);
        assert_(*pyramid.coarsest_image().ptr() == 42);
        assert_(pyramid.coarsest_greyscale().type() == CV_8UC1);
    }
    pyramid.init(make_image(SMALL_PYRAMID_SIZE));
    {
        assert_(pyramid.max_levels == 4);
        assert_(pyramid.depth() == 1);
        assert_(pyramid.image().cols == SMALL_PYRAMID_SIZE);
        assert_(pyramid.coarsest_image().cols == SMALL_PYRAMID_SIZE);
        assert_(pyramid.coarsest_image().type() == CV_8UC3);
        assert_(*pyramid.coarsest_image().ptr() == 42);
        assert_(pyramid.coarsest_greyscale().type() == CV_8UC1);
    }
}


void test_get_matching_level()
{
    ImagePyramid pyramid;
    const int current_level_big = 1;
    const int current_level_small = 0;
    const float desired_roi_width = 100.;
    const std::vector<float> test_roi_widths = {
        10000., 800., 410., 380., 310., 290., 210., 190., 110., 90., 55., 45., 10., 1., 0.
    };

    pyramid.init(make_image(BIG_PYRAMID_SIZE));
    for (float test_width : test_roi_widths)
    {
        auto [factor, depth] = get_matching_level(pyramid, current_level_big, test_width, desired_roi_width);
        assert_(depth >= 0);
        assert_(depth < pyramid.depth());
        //std::cerr << "width: " << test_width << " depth: " << depth << ", factor: " << factor << std::endl;
        if (test_width >= desired_roi_width / 2)
        {
            // Only allow downscaling from the selected pyramid level to the desired width
            assert_(factor * test_width >= desired_roi_width);
            assert_((factor == 0.25f) || (factor == 0.5f) || (factor == 1.0f) || (factor == 2.0f));
        }
        else
        {
            assert_(depth == 0);
            assert_(factor == 2.f);
        }
    }

    pyramid.init(make_image(SMALL_PYRAMID_SIZE));
    for (float test_width : test_roi_widths)
    {
        auto [factor, depth] = get_matching_level(pyramid, current_level_small, test_width, desired_roi_width);
        assert_(depth == 0);
        assert_(factor == 1.f);
    }
}


void run()
{
    test_find_input_intensity_quantile();
    test_normalize_brightness();
    test_image_pyramid();
    test_get_matching_level();
}

} // namespace neuralnet_tracker_tests