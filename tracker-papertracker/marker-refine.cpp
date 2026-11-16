/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

/*
 * AprilTag Library Notice
 *
 * The marker edge refinement algorithm used here has been adapted from the AprilTag
 * library. It is not a verbatim copy, but to the degree that it may constitute a
 * derivative work the following copyright notice and license apply:
 *
 * Copyright (C) 2013-2016, The Regents of The University of Michigan.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the Regents of The University of Michigan.
 */
#include "marker-refine.h"
#include <cmath>
#include <algorithm>

namespace papertracker {
    namespace {
        inline double magnitude(double x, double y) {
            return std::sqrt(x * x + y * y);
        }

        /* Bilinear sample of an 8-bit grayscale image using OpenCV's pixel-center convention
           (pixel (i, j) is centered at integer coordinates). Returns false if out of bounds.
        */
        inline bool sample_bilinear(const cv::Mat &gray, double x, double y, double &value)
        {
            const double xf = std::floor(x);
            const double yf = std::floor(y);
            const int xi = (int) xf;
            const int yi = (int) yf;

            if (xi < 0 || yi < 0 || xi + 1 >= gray.cols || yi + 1 >= gray.rows)
                return false;

            const double a = x - xf;
            const double b = y - yf;
            const uchar *row0 = gray.ptr<uchar>(yi);
            const uchar *row1 = gray.ptr<uchar>(yi + 1);

            value = (1 - a) * (1 - b) * row0[xi] + a * (1 - b) * row0[xi + 1] +
                    (1 - a) * b * row1[xi] + a * b * row1[xi + 1];

            return true;
        }

        /* Line in point-normal form: passes through (px, py) with unit normal (nx, ny).
        */
        struct edge_line {
            double px;
            double py;
            double nx;
            double ny;
        };

        /* Per-edge fit statistics.
        */
        struct edge_stats {
            double mse;      // mean squared perpendicular distance of edge points from the fitted line (px^2)
            double coverage; // fraction of samples that produced an edge point
            double contrast; // mean peak gradient (g1 - g2) over samples that produced an edge point
        };

        /* Fit a line to subpixel edge points found by searching along the normal of segment a->b.
           (onx, ony) is the unit normal pointing from the dark marker border toward the light margin.
           Samples whose peak gradient is below min_sample_contrast are treated as having no edge (e.g. occluded),
           so that they neither contribute noise to the line fit nor count towards coverage.
        */
        bool fit_edge(const cv::Mat &gray, const cv::Point2d &a, const cv::Point2d &b, double onx, double ony, double range, double min_sample_contrast, edge_line &line, edge_stats &stats)
        {
            const double len = magnitude(b.x - a.x, b.y - a.y);
            const int nsamples = std::max(16, (int)(len / 8));

            constexpr int steps_per_unit = 4;
            constexpr double step_length = 1.0 / steps_per_unit;
            constexpr double g_range = 1.0; // half-distance over which the gradient is measured.
            const int max_steps = (int)(2 * steps_per_unit * range) + 1;

            double Mx = 0;
            double My = 0;
            double Mxx = 0;
            double Mxy = 0;
            double Myy = 0;
            double N = 0;
            double contrast_sum = 0;

            for (int s = 0; s < nsamples; s++) {
                // Sample along the edge, avoiding the corners themselves as they are least reliable.
                const double alpha = (1.0 + s) / (nsamples + 1);
                const double x0 = alpha * a.x + (1 - alpha) * b.x;
                const double y0 = alpha * a.y + (1 - alpha) * b.y;

                double Mn = 0;
                double Mcount = 0;
                double peak = 0;

                for (int step = 0; step < max_steps; ++step) {
                    const double n = -range + step_length * step;

                    double g1;
                    double g2;
                    if (!sample_bilinear(gray, x0 + (n + g_range) * onx, y0 + (n + g_range) * ony, g1) ||
                        !sample_bilinear(gray, x0 + (n - g_range) * onx, y0 + (n - g_range) * ony, g2))
                        continue;

                    // Expect light on the outside, dark on the inside. Reject reversed gradients.
                    if (g1 < g2)
                        continue;

                    const double weight = (g1 - g2) * (g1 - g2);
                    Mn += weight * n;
                    Mcount += weight;

                    peak = std::max(peak, g1 - g2);
                }

                if (Mcount == 0 || peak < min_sample_contrast)
                    continue;

                const double n0 = Mn / Mcount;
                const double bx = x0 + n0 * onx;
                const double by = y0 + n0 * ony;

                Mx += bx;
                My += by;
                Mxx += bx * bx;
                Mxy += bx * by;
                Myy += by * by;
                N++;

                contrast_sum += peak;
            }

            // Need enough edge points for a meaningful fit.
            if (N < 4)
                return false;

            const double Ex = Mx / N, Ey = My / N;
            const double Cxx = Mxx / N - Ex * Ex;
            const double Cxy = Mxy / N - Ex * Ey;
            const double Cyy = Myy / N - Ey * Ey;

            // Normal is the eigenvector of the covariance matrix with the smallest eigenvalue.
            const double normal_theta = 0.5 * std::atan2(-2 * Cxy, Cyy - Cxx);

            line.px = Ex;
            line.py = Ey;
            line.nx = std::cos(normal_theta);
            line.ny = std::sin(normal_theta);

            // The smallest eigenvalue of the covariance matrix is the mean squared perpendicular distance
            // of the points from the fitted line.
            const double half_trace = 0.5 * (Cxx + Cyy);
            const double half_diff = 0.5 * (Cxx - Cyy);

            stats.mse = std::max(0.0, half_trace - std::sqrt(half_diff * half_diff + Cxy * Cxy));
            stats.coverage = N / nsamples;
            stats.contrast = contrast_sum / N;

            return true;
        }

        /* Intersect two lines in point-normal form.
        */
        bool intersect(const edge_line &l1, const edge_line &l2, cv::Point2d &out)
        {
            // Direction of each line is perpendicular to its normal: d = (-ny, nx).
            // Solve p1 + t*d1 = p2 + u*d2.
            const double d1x = -l1.ny;
            const double d1y = l1.nx;
            const double d2x = -l2.ny;
            const double d2y = l2.nx;
            const double det = d1x * (-d2y) - d1y * (-d2x);

            if (std::fabs(det) < 1e-3)
                return false;

            const double rx = l2.px - l1.px, ry = l2.py - l1.py;
            const double t = (rx * (-d2y) - ry * (-d2x)) / det;

            out.x = l1.px + t * d1x;
            out.y = l1.py + t * d1y;

            return true;
        }

        /* Refine the four corners of a detected marker using intensity gradients in the grayscale image.
           Each edge is re-estimated from subpixel gradient peaks found by searching up to `range` pixels
           along the edge normal, and corners are recomputed as intersections of the refined edge lines.
           The marker is assumed to have a dark border surrounded by a light background.

           Sample points whose peak gradient is below `min_sample_contrast` (gray levels) are ignored.

           Returns false (leaving the corners unchanged) if refinement could not be performed reliably.
        */
        bool refine_marker_edges(const cv::Mat &gray, std::array<cv::Point2f, 4> &corners, double range, double min_sample_contrast, marker_edge_quality &quality)
        {
            cv::Point2d p[4];
            cv::Point2d centroid(0, 0);

            for (int i = 0; i < 4; i++) {
                p[i] = cv::Point2d(corners[i].x, corners[i].y);
                centroid += p[i];
            }
            centroid /= 4;

            edge_line lines[4];
            edge_stats stats[4];

            for (size_t edge = 0; edge < 4; edge++) {
                const cv::Point2d &a = p[edge];
                const cv::Point2d &b = p[(edge + 1) % 4];

                double nx = b.y - a.y;
                double ny = -(b.x - a.x);
                const double mag = magnitude(nx, ny);

                if (mag < 1e-6)
                    return false;

                nx /= mag;
                ny /= mag;

                // Orient the normal outward (away from the marker center), i.e. toward the light margin.
                const cv::Point2d mid = (a + b) * 0.5;
                if ((mid.x - centroid.x) * nx + (mid.y - centroid.y) * ny < 0) {
                    nx = -nx;
                    ny = -ny;
                }

                if (!fit_edge(gray, a, b, nx, ny, range, min_sample_contrast, lines[edge], stats[edge]))
                    return false;
            }

            // Corner i lies at the intersection of edge (i-1) and edge i.
            cv::Point2d refined[4];

            for (int i = 0; i < 4; i++) {
                if (!intersect(lines[(i + 3) & 3], lines[i], refined[i]))
                    return false;

                // Sanity check: refinement should only nudge corners. Large jumps indicate a bad fit
                // (e.g. an edge latched onto a neighboring feature).
                const double max_shift = 2.0 * range + 1.0;
                if (magnitude(refined[i].x - p[i].x, refined[i].y - p[i].y) > max_shift)
                    return false;
            }

            for (int i = 0; i < 4; i++)
                corners[i] = cv::Point2f((float)refined[i].x, (float)refined[i].y);

            quality.max_line_fit_mse = std::max({ stats[0].mse, stats[1].mse, stats[2].mse, stats[3].mse });
            quality.min_edge_coverage = std::min({ stats[0].coverage, stats[1].coverage, stats[2].coverage, stats[3].coverage });
            quality.min_edge_contrast = std::min({ stats[0].contrast, stats[1].contrast, stats[2].contrast, stats[3].contrast });

            return true;
        }
    }

    /* Refine marker edges a maximum of `iterations` times. If `quality` is not null it receives quality
       measures from the last successful iteration.
    */
    bool refine_marker_edges(const cv::Mat &gray, std::array<cv::Point2f, 4> &corners, double range, double min_sample_contrast, marker_edge_quality *quality, int iterations)
    {
        CV_Assert(gray.type() == CV_8UC1);

        bool any = false;
        marker_edge_quality q;

        for (int it = 0; it < iterations; it++) {
            if (!refine_marker_edges(gray, corners, range, min_sample_contrast, q))
                break;

            any = true;

            if (quality)
                *quality = q;
        }

        return any;
    }

    /* Check that every interior angle of the quad is at least `angle_limit_radians` away from 0 and 180 degrees
       and that the quad is convex. Near-degenerate quads produce unreliable pose estimates.
    */
    bool marker_quad_angles_ok(const std::array<cv::Point2f, 4> &corners, double angle_limit_radians)
    {
        // abs(cos(angle)) must not exceed cos(angle_limit_radians): the angle must lie within [min_angle, pi - min_angle].
        const double max_abs_cos = std::cos(angle_limit_radians);
        double cross_sign = 0;

        for (int i = 0; i < 4; i++) {
            const cv::Point2d p(corners[i].x, corners[i].y);
            const cv::Point2d prev(corners[(i + 3) & 3].x, corners[(i + 3) & 3].y);
            const cv::Point2d next(corners[(i + 1) & 3].x, corners[(i + 1) & 3].y);

            const cv::Point2d u = prev - p, v = next - p;
            const double lu = magnitude(u.x, u.y);
            const double lv = magnitude(v.x, v.y);

            if (lu < 1e-6 || lv < 1e-6)
                return false;

            if (std::fabs(u.dot(v) / (lu * lv)) > max_abs_cos)
                return false;

            // All corners must turn the same way for the quad to be convex.
            const double cross = u.cross(v);

            if (i == 0)
                cross_sign = cross;
            else if (cross * cross_sign <= 0)
                return false;
        }

        return true;
    }
}
