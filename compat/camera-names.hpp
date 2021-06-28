/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <vector>
#include <QString>
#   include <tuple>

#include "export.hpp"

OTR_COMPAT_EXPORT std::vector<std::tuple<QString, int>> get_camera_names();
OTR_COMPAT_EXPORT int camera_name_to_index(const QString &name);

