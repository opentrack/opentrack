#pragma once

#include "export.hpp"
#include "macros.hpp"

#include <QWidget>

cc_noinline OTR_COMPAT_EXPORT
void set_is_visible(QWidget const& w, bool force = false);

cc_noinline OTR_COMPAT_EXPORT
bool check_is_visible();
