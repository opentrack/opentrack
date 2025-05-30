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
#include "compat/math-imports.hpp"

#include <opencv2/imgproc.hpp>

#undef PREVIEW
//#define PREVIEW

#if defined PREVIEW
#   include <opencv2/highgui.hpp>
#endif

#include <cmath>
#include <algorithm>
#include <memory>

#include <QDebug>

using namespace numeric_types;

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
static vec2 MeanShiftIteration(const cv::Mat1b &frame_gray, const vec2 &current_center, f filter_width)
{
    const f s = 1 / filter_width;

    f m = 0;
    vec2 com { 0, 0 };
    for (int i = 0; i < frame_gray.rows; i++)
    {
        uint8_t const* const __restrict frame_ptr = frame_gray.ptr(i);
        for (int j = 0; j < frame_gray.cols; j++)
        {
            f val = frame_ptr[j];
            val = val * val; // taking the square weighs brighter parts of the image stronger.
            f dx = (j - current_center[0])*s;
            f dy = (i - current_center[1])*s;
            f max = std::fmax(f(0), 1 - dx*dx - dy*dy);
            val *= max;
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
    for (cv::Mat1b& x : ch)
        x.create(orig_frame.rows, orig_frame.cols);
}

void PointExtractor::ensure_buffers(const cv::Mat& frame)
{
    const int W = frame.cols, H = frame.rows;

    frame_gray.create(H, W);
    frame_bin.create(H, W);
}

void PointExtractor::extract_single_channel(const cv::Mat& orig_frame, int idx, cv::Mat1b& dest)
{
    ensure_channel_buffers(orig_frame);

    const int from_to[] = {
        idx, 0,
    };

    cv::mixChannels(&orig_frame, 1, &dest, 1, from_to, 1);
}

void PointExtractor::filter_single_channel(const cv::Mat& orig_frame, float r, float g, float b, bool overexp, cv::Mat1b& dest)
{
    ensure_channel_buffers(orig_frame);

    // just filter for colour or also include overexposed regions?
    if (!overexp)
        cv::transform(orig_frame, dest, cv::Mat(cv::Matx13f(b, g, r)));
    else
    {
        for (int i = 0; i < orig_frame.rows; i++)
        {
            cv::Vec3b const* const __restrict orig_ptr = orig_frame.ptr<cv::Vec3b>(i);
            uint8_t* const __restrict dest_ptr = dest.ptr(i);
            for (int j = 0; j < orig_frame.cols; j++)
            {
                // get the intensity of the key color (i.e. +ve coefficients)
                uchar blue = orig_ptr[j][0], green = orig_ptr[j][1], red = orig_ptr[j][2];
                float key = std::max(b, 0.0f) * blue + std::max(g, 0.0f) * green + std::max(r, 0.0f) * red;
                // get the intensity of the non-key color (i.e. -ve coefficients)
                float nonkey = std::max(-b, 0.0f) * blue + std::max(-g, 0.0f) * green + std::max(-r, 0.0f) * red;
                // the result is key color minus non-key color inversely weighted by key colour intensity
                dest_ptr[j] = std::max(0.0f, std::min(255.0f, key - (255.0f - key) / 255.0f * nonkey));
            }
        }
    }
}

void PointExtractor::color_to_grayscale(const cv::Mat& frame, cv::Mat1b& output)
{
    if (frame.channels() == 1)
    {
        output.create(frame.rows, frame.cols);
        frame.copyTo(output);
        return;
    }

    const float half_chr_key_str = *s.chroma_key_strength * 0.5;
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
    case pt_color_red_chromakey:
    {
        filter_single_channel(frame, 1, -half_chr_key_str, -half_chr_key_str, s.chroma_key_overexposed, output);
        break;
    }
    case pt_color_green_chromakey:
    {
        filter_single_channel(frame, -half_chr_key_str, 1, -half_chr_key_str, s.chroma_key_overexposed, output);
        break;
    }
    case pt_color_blue_chromakey:
    {
        filter_single_channel(frame, -half_chr_key_str, -half_chr_key_str, 1, s.chroma_key_overexposed, output);
        break;
    }
    case pt_color_cyan_chromakey:
    {
        filter_single_channel(frame, -*s.chroma_key_strength, 0.5, 0.5, s.chroma_key_overexposed, output);
        break;
    }
    case pt_color_yellow_chromakey:
    {
        filter_single_channel(frame, 0.5, 0.5, -*s.chroma_key_strength, s.chroma_key_overexposed, output);
        break;
    }
    case pt_color_magenta_chromakey:
    {
        filter_single_channel(frame, 0.5, -*s.chroma_key_strength, 0.5, s.chroma_key_overexposed, output);
        break;
    }
    case pt_color_hardware:
        eval_once(qDebug() << "camera driver doesn't support grayscale");
        goto do_grayscale;
    default:
        eval_once(qDebug() << "wrong pt_color_type enum value" << int(s.blob_color));
    [[fallthrough]];
    case pt_color_bt709:
do_grayscale:
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

        const f radius = threshold_radius_value(frame_gray.cols, frame_gray.rows, threshold_slider_value);

        float const* const __restrict ptr = hist.ptr<float>(0);
        const unsigned area = unsigned(iround(3 * pi * radius*radius));
        const unsigned sz = unsigned(hist.cols * hist.rows);
        unsigned thres = 1;
        for (unsigned i = sz-1, cnt = 0; i > 1; i--)
        {
            cnt += (unsigned)ptr[i];
            if (cnt >= area)
                break;
            thres = i;
        }

        cv::threshold(frame_gray, output, thres, 255, cv::THRESH_BINARY);
    }
}

static void draw_blobs(cv::Mat& preview_frame, const blob* blobs, unsigned nblobs, const cv::Size& size)
{
    for (unsigned k = 0; k < nblobs; k++)
    {
        const blob& b = blobs[k];

        if (b.radius < 0)
            continue;

        const f dpi = preview_frame.cols / f(320);
        const f offx = 10 * dpi, offy = f(7.5) * dpi;

        const f cx = preview_frame.cols / f(size.width),
                cy = preview_frame.rows / f(size.height),
                c  = std::fmax(f(1), cx+cy)/2;

        cv::Point p(iround(b.pos[0] * cx), iround(b.pos[1] * cy));

        auto outline_color = k >= PointModel::N_POINTS
                             ? cv::Scalar(192, 192, 192)
                             : cv::Scalar(255, 255, 0);

        cv::ellipse(preview_frame, p,
                    {iround(b.rect.width/(f)2+2*c), iround(b.rect.height/(f)2+2*c)},
                    0, 0, 360, outline_color, iround(dpi), cv::LINE_AA);

        char buf[16];
        std::snprintf(buf, sizeof(buf), "%.2fpx", (double)b.radius);

        auto text_color = k >= PointModel::N_POINTS
                          ? cv::Scalar(160, 160, 160)
                          : cv::Scalar(0, 0, 255);

        cv::Point pos(iround(b.pos[0]*cx+offx), iround(b.pos[1]*cy+offy));
        cv::putText(preview_frame, buf, pos,
                    cv::FONT_HERSHEY_PLAIN, iround(dpi), text_color,
                    1);
    }
}

static vec2 meanshift_initial_guess(const cv::Rect rect, cv::Mat& frame_roi)
{
    vec2 ret = {rect.width/(f)2, rect.height/(f)2};

    // compute center initial guess
    double ynorm = 0, xnorm = 0, y = 0, x = 0;
    for (int j = 0; j < rect.height; j++)
    {
        const unsigned char* __restrict ptr = frame_roi.ptr<unsigned char>(j);
        for (int i = 0; i < rect.width; i++)
        {
            double val = ptr[i] * 1./255;
            x += i * val;
            y += j * val;
            xnorm += val;
            ynorm += val;
        }
    }
    constexpr double eps = 1e-4;
    if (xnorm > eps && ynorm > eps)
        ret = { (f)(x / xnorm), (f)(y / ynorm) };
    return ret;
}

void PointExtractor::extract_points(const pt_frame& frame_,
                                    pt_preview& preview_frame_,
                                    bool preview_visible,
                                    std::vector<vec2>& points)
{
    const cv::Mat& frame = frame_.as_const<Frame>()->mat;

    ensure_buffers(frame);
    color_to_grayscale(frame, frame_gray);

#if defined PREVIEW
    cv::imshow("capture", frame_gray);
    cv::waitKey(1);
#endif

    threshold_image(frame_gray, frame_bin);

    const f region_size_min = (f)s.min_point_size;
    const f region_size_max = (f)s.max_point_size;

    unsigned idx = 0;

    blobs.clear();

    for (int y=0; y < frame_bin.rows; y++)
    {
        const unsigned char* __restrict ptr_bin = frame_bin.ptr(y);
        for (int x=0; x < frame_bin.cols; x++)
        {
            if (ptr_bin[x] != 255)
                continue;
            idx = (unsigned)(blobs.size() + 1);

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

            const f radius = std::sqrt((f)cnt) / std::sqrt(pi);
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
            //break;
        }
    }
end:

    const int W = frame_gray.cols;
    const int H = frame_gray.rows;

    const unsigned sz = (unsigned)blobs.size();

    std::sort(blobs.begin(), blobs.end(), [](const blob& b1, const blob& b2) { return b2.brightness < b1.brightness; });

    for (idx = 0; idx < sz; ++idx)
    {
        blob& b = blobs[idx];
        cv::Rect rect = b.rect & cv::Rect(0, 0, W, H); // crop at frame boundaries
        cv::Mat frame_roi = frame_gray(rect);

        // smaller values mean more changes. 1 makes too many changes while 1.5 makes about .1
        static constexpr f radius_c = f(1.75);

        const f kernel_radius = b.radius * radius_c;
        vec2 pos = meanshift_initial_guess(rect, frame_roi); // position relative to ROI.

        for (int iter = 0; iter < 10; ++iter)
        {
            vec2 com_new = MeanShiftIteration(frame_roi, pos, kernel_radius);
            vec2 delta = com_new - pos;
            pos = com_new;
            if (delta.dot(delta) < f(1e-3))
                break;
        }

        b.pos[0] = pos[0] + rect.x;
        b.pos[1] = pos[1] + rect.y;
    }

    if (preview_visible)
        draw_blobs(preview_frame_.as<Frame>()->mat,
                   blobs.data(), (unsigned)blobs.size(),
                   frame_gray.size());


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
