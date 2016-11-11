/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "affine.hpp"

Affine::Affine() : R(mat33::eye()), t(0,0,0) {}

Affine::Affine(const mat33& R, const vec3& t) : R(R),t(t) {}

Affine operator*(const Affine& X, const Affine& Y)
{
    return Affine(X.R*Y.R, X.R*Y.t + X.t);
}

Affine operator*(const mat33& X, const Affine& Y)
{
    return Affine(X*Y.R, X*Y.t);
}

Affine operator*(const Affine& X, const mat33& Y)
{
    return Affine(X.R*Y, X.t);
}

vec3 operator*(const Affine& X, const vec3& v)
{
    return X.R*v + X.t;
}
