#pragma once

#include "macros.hpp"
#include "export.hpp"

#include <QString>

OTR_COMPAT_EXPORT cc_noinline
const QString& application_base_path();

#define OPENTRACK_BASE_PATH (application_base_path())
