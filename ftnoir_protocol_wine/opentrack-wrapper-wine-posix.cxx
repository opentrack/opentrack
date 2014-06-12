#define OPENTRACK_COMPAT_BUNDLED
#ifdef _WIN32
#   undef _WIN32
#endif

#define PortableLockedShm ShmPosix
#include "compat/compat.h"
#include "compat/compat.cpp"
