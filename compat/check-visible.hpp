#pragma once

#include "export.hpp"
#include "util.hpp"

#include <QWidget>

never_inline OTR_COMPAT_EXPORT void set_is_visible(QWidget const& w, bool force = false);
never_inline OTR_COMPAT_EXPORT bool check_is_visible();
