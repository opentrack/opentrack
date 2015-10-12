#include "opentrack-compat/export.hpp"

#ifdef __cplusplus
extern "C"
#endif
OPENTRACK_EXPORT
const char* opentrack_version;

const char* opentrack_version = OPENTRACK_VERSION;
