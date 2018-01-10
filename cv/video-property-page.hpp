#pragma once

#include <QString>

#ifdef _WIN32
#   include <windows.h>
#endif

#include <opencv2/videoio.hpp>

struct IBaseFilter;

struct video_property_page final
{
    video_property_page() = delete;
    static bool show(int id);
    static bool show_from_capture(cv::VideoCapture& cap, int index);
private:
#ifdef _WIN32
    static IBaseFilter* get_device(int id);
    static cv::VideoCapture last_capture;
#endif
};
