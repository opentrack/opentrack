/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include "meanvector.h"

namespace arucohead {
    struct Marker
    {
        int id;
        MeanVector rvec_local;
        MeanVector tvec_local;

        Marker(): id(0)
        {}

        Marker(int id, const MeanVector &rvec, const MeanVector &tvec) : id(id), rvec_local(rvec), tvec_local(tvec)
        {}
    };
}
