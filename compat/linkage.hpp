// generates export.hpp for each module from compat/linkage.hpp

#pragma once

#include "compat/linkage-macros.hpp"

#ifdef BUILD_${module}
#   define OTR_${module}_EXPORT OTR_GENERIC_EXPORT
#else
#   define OTR_${module}_EXPORT OTR_GENERIC_IMPORT
#endif
