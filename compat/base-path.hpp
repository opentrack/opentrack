#pragma once

#include "export.hpp"

#include <QString>

OTR_COMPAT_EXPORT
const QString& application_base_path();

#define OPENTRACK_BASE_PATH (application_base_path())

