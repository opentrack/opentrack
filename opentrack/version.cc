#include "opentrack/export.hpp"

#ifdef __cplusplus
extern "C"
#endif
OPENTRACK_EXPORT
volatile const char* opentrack_version;

volatile const char* opentrack_version = OPENTRACK_VERSION;
