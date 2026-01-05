#pragma once

#include "export.hpp"
#include "macros.h"

class QWidget;

tr_never_inline OTR_COMPAT_EXPORT
void set_is_visible(QWidget const& w, bool force = false);

OTR_COMPAT_EXPORT
bool check_is_visible();

OTR_COMPAT_EXPORT
void force_is_visible(bool value);
