// generates export.hpp for each module from compat/linkage.hpp

#pragma once

#include "compat/linkage-macros.hpp"

#ifdef BUILD_MIGRATION
#   define OTR_MIGRATION_EXPORT OTR_GENERIC_EXPORT
#else
#   define OTR_MIGRATION_EXPORT OTR_GENERIC_IMPORT
#endif
