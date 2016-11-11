/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <opencv2/core.hpp>
#include "numeric.hpp"
using namespace types;



class Affine final
{
public:
    Affine();
    Affine(const mat33& R, const vec3& t);

    mat33 R;
    vec3 t;
};

Affine operator*(const Affine& X, const Affine& Y);
Affine operator*(const mat33& X, const Affine& Y);
Affine operator*(const Affine& X, const mat33& Y);
vec3 operator*(const Affine& X, const vec3& v);
