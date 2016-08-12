#ifdef _WIN32
#   undef _WIN32
#endif

#define PortableLockedShm ShmPosix
#include "compat/shm.h"
#include "compat/shm.cpp"
