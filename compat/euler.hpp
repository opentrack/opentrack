#pragma once

#include "export.hpp"
#include "simple-mat.hpp"

namespace euler {

template<int h_, int w_> using dmat = Mat<double, h_, w_>;

using dvec2 = Mat<double, 2, 1>;
using dvec3 = Mat<double, 3, 1>;

using rmat = dmat<3, 3>;
using euler_t = dmat<3, 1>;

rmat OPENTRACK_COMPAT_EXPORT euler_to_rmat(const euler_t& input);

euler_t OPENTRACK_COMPAT_EXPORT rmat_to_euler(const rmat& R);

void OPENTRACK_COMPAT_EXPORT tait_bryan_to_matrices(const euler_t& input,
                                                   rmat& r_roll,
                                                   rmat& r_pitch,
                                                   rmat& r_yaw);

} // end ns euler
