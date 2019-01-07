/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2015-2017 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "point_extractor.h"
#include "point_tracker.h"
#include "frame.hpp"

#include "cv/numeric.hpp"
#include "compat/math.hpp"

#include <opencv2/videoio.hpp>

#undef PREVIEW
//#define PREVIEW

#if defined PREVIEW
#   include <opencv2/highgui.hpp>
#endif

#include <cmath>
#include <algorithm>
#include <cinttypes>
#include <memory>

#include <QDebug>

using namespace types;

// meanshift code written by Michael Welter

/*
http://en.wikipedia.org/wiki/Mean-shift
In this application the idea, is to eliminate any bias of the point estimate 
which is introduced by the rather arbitrary thresholded area. One must recognize
that the thresholded area can only move in one pixel increments since it is
binary. Thus, its center of mass might make "jumps" as pixels are added/removed
from the thresholded area.
With mean-shift, a moving "window" or kernel is multiplied with the gray-scale
image, and the COM is calculated of the result. This is iterated where the
kernel center is set the previously computed COM. Thus, peaks in the image intensity
distribution "pull" the kernel towards themselves. Eventually it stops moving, i.e.
then the computed COM coincides with the kernel center. We hope that the 
corresponding location is a good candidate for the extracted point.
The idea similar to the window scaling suggested in  Berglund et al. "Fast, bias-free 
algorithm for tracking single particles with variable size and shape." (2008).
*/
static cv::Vec2d MeanShiftIteration(const cv::Mat &frame_gray, const vec2 &current_center, f filter_width)
{
    // Most amazingly this function runs faster with doubles than with floats.
    const f s = 1 / filter_width;

    f m = 0;
    vec2 com { 0, 0  };
    for (int i = 0; i < frame_gray.rows; i++)
    {
        auto frame_ptr = (uint8_t const* __restrict)frame_gray.ptr(i);
        for (int j = 0; j < frame_gray.cols; j++)
        {
            f val = frame_ptr[j];
            val = val * val; // taking the square weighs brighter parts of the image stronger.
            {
                f dx = (j - current_center[0])*s;
                f dy = (i - current_center[1])*s;
                f max = std::fmax(f{0}, 1 - dx*dx - dy*dy);
                val *= max;
            }
            m += val;
            com[0] += j * val;
            com[1] += i * val;
        }
    }
    if (m > f(.1))
    {
        com *= 1 / m;
        return com;
    }
    else
        return current_center;
}

namespace pt_module {

PointExtractor::PointExtractor(const QString& module_name) : s(module_name)
{
    blobs.reserve(max_blobs);
}

void PointExtractor::ensure_channel_buffers(const cv::Mat& orig_frame)
{
    if (ch[0].rows != orig_frame.rows || ch[0].cols != orig_frame.cols)
        for (unsigned k = 0; k < 3; k++) // NOLINT(modernize-loop-convert)
            ch[k] = cv::Mat1b(orig_frame.rows, orig_frame.cols);
}

void PointExtractor::ensure_buffers(const cv::Mat& frame)
{
    const int W = frame.cols, H = frame.rows;

    if (frame_gray.rows != W || frame_gray.cols != H)
    {
        frame_gray = cv::Mat1b(H, W);
        frame_bin = cv::Mat1b(H, W);
        frame_gray_unmasked = cv::Mat1b(H, W);
    }
}

void PointExtractor::extract_single_channel(const cv::Mat& orig_frame, int idx, cv::Mat& dest)
{
    ensure_channel_buffers(orig_frame);

    const int from_to[] = {
        idx, 0,
    };

    cv::mixChannels(&orig_frame, 1, &dest, 1, from_to, 1);
}

void PointExtractor::color_to_grayscale(const cv::Mat& frame, cv::Mat1b& output)
{
    switch (s.blob_color)
    {
    case pt_color_green_only:
    {
        extract_single_channel(frame, 1, output);
        break;
    }
    case pt_color_blue_only:
    {
        extract_single_channel(frame, 0, output);
        break;
    }
    case pt_color_red_only:
    {
        extract_single_channel(frame, 2, output);
        break;
    }
    case pt_color_average:
    {
        const int W = frame.cols, H = frame.rows, sz = W*H;
        cv::reduce(frame.reshape(1, sz),
                   output.reshape(1, sz),
                   1, cv::REDUCE_AVG);
        break;
    }
    default:
        eval_once(qDebug() << "wrong pt_color_type enum value" << int(s.blob_color));
    [[fallthrough]];
    case pt_color_natural:
        cv::cvtColor(frame, output, cv::COLOR_BGR2GRAY);
        break;
    }
}

void PointExtractor::threshold_image(const cv::Mat& frame_gray, cv::Mat1b& output)
{
    const int threshold_slider_value = s.threshold_slider.to<int>();

    if (!s.auto_threshold)
    {
        cv::threshold(frame_gray, output, threshold_slider_value, 255, cv::THRESH_BINARY);
    }
    else
    {
        const int hist_size = 256;
        const float ranges_[] = { 0, 256 };
        float const* ranges = (const float*) ranges_;

        cv::calcHist(&frame_gray,
                     1,
                     nullptr,
                     cv::noArray(),
                     hist,
                     1,
                     &hist_size,
                     &ranges);

        const f radius = (f) threshold_radius_value(frame_gray.cols, frame_gray.rows, threshold_slider_value);

        float const* const __restrict ptr = hist.ptr<float>(0);
        const unsigned area = uround(3 * pi * radius*radius);
        const unsigned sz = unsigned(hist.cols * hist.rows);
        constexpr unsigned min_thres = 64;
        unsigned thres = min_thres;
        for (unsigned i = sz-1, cnt = 0; i > 32; i--)
        {
            cnt += (unsigned)ptr[i];
            if (cnt >= area)
                break;
            thres = i;
        }

        if (thres > min_thres)
            thres = uround(thres * .8);

        cv::threshold(frame_gray, output, thres, 255, cv::THRESH_BINARY);
    }
}

void PointExtractor::extract_points(const pt_frame& frame_, pt_preview& preview_frame_, std::vector<vec2>& points)
{
    const cv::Mat& frame = frame_.as_const<Frame>()->mat;
    cv::Mat& preview_frame = *preview_frame_.as<Preview>();

    ensure_buffers(frame);
    color_to_grayscale(frame, frame_gray_unmasked);

#if defined PREVIEW
    cv::imshow("capture", frame_gray);
    cv::waitKey(1);
#endif

    threshold_image(frame_gray_unmasked, frame_bin);
    frame_gray_unmasked.copyTo(frame_gray, frame_bin);

    const f region_size_min = (f)s.min_point_size;
    const f region_size_max = (f)s.max_point_size;

    unsigned idx = 0;

    blobs.clear();

    for (int y=0; y < frame_bin.rows; y++)
    {
        const unsigned char* ptr_bin = frame_bin.ptr(y);
        for (int x=0; x < frame_bin.cols; x++)
        {
            if (ptr_bin[x] != 255)
                continue;
            idx = blobs.size() + 1;

            cv::Rect rect;
            cv::floodFill(frame_bin,
                          cv::Point(x,y),
                          cv::Scalar(idx),
                          &rect,
                          cv::Scalar(0),
                          cv::Scalar(0),
                          4 | cv::FLOODFILL_FIXED_RANGE);

            unsigned cnt = 0;
            unsigned norm = 0;

            const int ymax = rect.y+rect.height,
                      xmax = rect.x+rect.width;

            for (int i=rect.y; i < ymax; i++)
            {
                unsigned char const* const __restrict ptr_blobs = frame_bin.ptr(i);
                unsigned char const* const __restrict ptr_gray = frame_gray.ptr(i);
                for (int j=rect.x; j < xmax; j++)
                {
                    if (ptr_blobs[j] != idx)
                        continue;

                    //ptr_blobs[j] = 0;
                    norm += ptr_gray[j];
                    cnt++;
                }
            }

            const double radius = std::sqrt(cnt / pi);
            if (radius > region_size_max || radius < region_size_min)
                continue;

            blobs.emplace_back(radius,
                               vec2(rect.width/f(2), rect.height/f(2)),
                               std::pow(f(norm), f(1.1))/cnt,
                               rect);

            if (idx >= max_blobs)
                goto end;

            // XXX we could go to the next scanline unless the points are really small.
            // i'd expect each point being present on at least one unique scanline
            // but it turns out some people are using 2px points -sh 20180110
#if defined BROKEN && 0
            break;
#endif
        }
    }
end:

    const int W = frame_gray.cols;
    const int H = frame_gray.rows;

    const unsigned sz = blobs.size();

    std::sort(blobs.begin(), blobs.end(), [](const blob& b1, const blob& b2) { return b2.brightness < b1.brightness; });

    for (idx = 0; idx < sz; ++idx)
    {
        blob& b = blobs[idx];
        cv::Rect rect = b.rect;

        rect.x -= rect.width / 2;
        rect.y -= rect.height / 2;
        rect.width *= 2;
        rect.height *= 2;
        rect &= cv::Rect(0, 0, W, H);  // crop at frame boundaries

        cv::Mat frame_roi = frame_gray(rect);

        // smaller values mean more changes. 1 makes too many changes while 1.5 makes about .1
        static constexpr f radius_c = f(1.75);

        const f kernel_radius = b.radius * radius_c;
        vec2 pos(rect.width/f{2}, rect.height/f{2}); // position relative to ROI.

        for (int iter = 0; iter < 10; ++iter)
        {
            vec2 com_new = MeanShiftIteration(frame_roi, pos, kernel_radius);
            vec2 delta = com_new - pos;
            pos = com_new;
            if (delta.dot(delta) < 1e-2)
                break;
        }

        b.pos[0] = pos[0] + rect.x;
        b.pos[1] = pos[1] + rect.y;
    }

    for (unsigned k = 0; k < blobs.size(); k++)
    {
        blob& b = blobs[k];

        const f dpi = preview_frame.cols / f(320);
        const f offx = 10 * dpi, offy = f{7.5} * dpi;

        const f cx = preview_frame.cols / f(frame.cols),
                cy = preview_frame.rows / f(frame.rows),
                c_ = (cx+cy)/2;

        static constexpr unsigned fract_bits = 16;
        static constexpr double c_fract(1 << fract_bits);

        cv::Point p(iround(b.pos[0] * cx * c_fract), iround(b.pos[1] * cy * c_fract));

        auto circle_color = k >= PointModel::N_POINTS
                            ? cv::Scalar(192, 192, 192)
                            : cv::Scalar(255, 255, 0);

        const f overlay_size = dpi > 1.5 ? 2 : 1;

        cv::circle(preview_frame, p, iround((b.radius + 3.3) * c_ * c_fract),
                   circle_color, (int)overlay_size,
                   cv::LINE_AA, fract_bits);

        char buf[16];
        buf[sizeof(buf)-1] = '\0';
        std::snprintf(buf, sizeof(buf) - 1, "%.2fpx", b.radius);

        auto text_color = k >= PointModel::N_POINTS
                          ? cv::Scalar(160, 160, 160)
                          : cv::Scalar(0, 0, 255);

        cv::Point pos(iround(b.pos[0]*cx+offx), iround(b.pos[1]*cy+offy));
        cv::putText(preview_frame, buf, pos,
                    cv::FONT_HERSHEY_PLAIN, overlay_size, text_color,
                    1);
    }

    // End of mean shift code. At this point, blob positions are updated with hopefully less noisy less biased values.
    points.reserve(max_blobs);
    points.clear();

    for (const auto& b : blobs)
    {
        // note: H/W is equal to fx/fy

        vec2 p;
        std::tie(p[0], p[1]) = to_screen_pos(b.pos[0], b.pos[1], W, H);
        points.push_back(p);
    }
}

blob::blob(f radius, const vec2& pos, f brightness, const cv::Rect& rect) :
    radius(radius), brightness(brightness), pos(pos), rect(rect)
{
    //qDebug() << "radius" << radius << "pos" << pos[0] << pos[1];
}

} // ns pt_module
