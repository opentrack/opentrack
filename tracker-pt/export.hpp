// generates export.hpp for each module from compat/linkage.hpp

#pragma once

#if 0
#   include "compat/linkage-macros.hpp"
#   ifdef BUILD_TRACKER_PT
#      define OTR_PT_EXPORT OTR_GENERIC_EXPORT
#   else
#      define OTR_PT_EXPORT OTR_GENERIC_IMPORT
#   endif
#else
// static link
#   define OTR_PT_EXPORT
#endif
