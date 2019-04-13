#pragma once

#include "pt-settings.hpp"

#include "cv/numeric.hpp"
#include "options/options.hpp"
#include "video/camera.hpp"

#include <tuple>
#include <type_traits>
#include <memory>

#include <opencv2/core.hpp>

#include <QImage>
#include <QString>

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wweak-vtables"
#endif

const int KPointCount = 3;

class IPointExtractor
{
public:
    using vec2 = numeric_types::vec2;
    using f = numeric_types::f;

    virtual void extract_points(const cv::Mat& image, cv::Mat* aPreview, std::vector<vec2>& aPoints) = 0;
};

struct IEasyTrackerTraits
{
    template<typename t> using pointer = std::shared_ptr<t>;

    IEasyTrackerTraits();
    virtual ~IEasyTrackerTraits();

    virtual pointer<IPointExtractor> make_point_extractor() const = 0;
    virtual QString get_module_name() const = 0;
};

template<typename t>
using pt_pointer = typename IEasyTrackerTraits::pointer<t>;

#ifdef __clang__
#   pragma clang diagnostic pop
#endif
