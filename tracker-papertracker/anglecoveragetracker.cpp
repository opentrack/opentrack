/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "anglecoveragetracker.h"

#include <cmath>

namespace papertracker {
    AngleCoverageBin::AngleCoverageBin(int pitch_index, int yaw_index)
        : pitch_index(pitch_index), yaw_index(yaw_index)
    {}

    bool AngleCoverageBin::operator==(const AngleCoverageBin &other) const {
        return pitch_index == other.pitch_index && yaw_index == other.yaw_index;
    }

    AngleCoverageTracker::AngleCoverageTracker(double pitch_step, double yaw_step)
        : pitch_step(pitch_step), yaw_step(yaw_step)
    {}

    void AngleCoverageTracker::add_visit(const AngleCoverageBin &bin) {
        ++visits[bin];
    }

    void AngleCoverageTracker::add_visit(double pitch, double yaw) {
        add_visit(get_bin(pitch, yaw));
    }

    void AngleCoverageTracker::clear_visits(const AngleCoverageBin &bin)
    {
        visits[bin] = 0;
    }

    void AngleCoverageTracker::clear_visits(double pitch, double yaw)
    {
        clear_visits(get_bin(pitch, yaw));
    }

    int AngleCoverageTracker::get_visit_count(const AngleCoverageBin &bin) const {
        if (visits.count(bin) == 0)
            return 0;

        return visits.at(bin);
    }

    int AngleCoverageTracker::get_visit_count(double pitch, double yaw) const {
        return get_visit_count(get_bin(pitch, yaw));
    }

    AngleCoverageBin AngleCoverageTracker::get_bin(double pitch, double yaw) const {
        int pitch_index = floor(pitch / pitch_step);
        int yaw_index = floor(yaw / yaw_step);

        return AngleCoverageBin(pitch_index, yaw_index);
    }
}