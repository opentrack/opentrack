#pragma once

/* Copyright (c) 2016-2018 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "export.hpp"
#include "simple-mat.hpp"

namespace euler {

template<int h_, int w_> using dmat = Mat<double, h_, w_>;

using dvec3 = Mat<double, 3, 1>;

using rmat = dmat<3, 3>;
using euler_t = dmat<3, 1>;

rmat OTR_COMPAT_EXPORT euler_to_rmat(const euler_t& input);
euler_t OTR_COMPAT_EXPORT rmat_to_euler(const rmat& R);

} // end ns euler
