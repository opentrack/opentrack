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

using dvec2 = Mat<double, 2, 1>;
using dvec3 = Mat<double, 3, 1>;

using rmat = dmat<3, 3>;
using euler_t = dmat<3, 1>;

rmat OTR_COMPAT_EXPORT euler_to_rmat(const euler_t& input);

euler_t OTR_COMPAT_EXPORT rmat_to_euler(const rmat& R);

void OTR_COMPAT_EXPORT tait_bryan_to_matrices(const euler_t& input,
                                                   rmat& r_roll,
                                                   rmat& r_pitch,
                                                   rmat& r_yaw);

rmat OTR_COMPAT_EXPORT quaternion_to_mat(const dmat<1, 4>& q);
//rmat OTR_COMPAT_EXPORT quaternion_to_mat(const dmat<4, 1>& q);

} // end ns euler
