#ifdef _WIN32
#   undef _WIN32
#endif

#define PortableLockedShm ShmPosix
#include "opentrack-compat/shm.h"
#include "opentrack-compat/shm.cpp"
