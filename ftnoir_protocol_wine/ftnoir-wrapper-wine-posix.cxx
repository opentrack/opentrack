#ifdef _WIN32
#undef _WIN32
#endif
#ifdef __WIN32
#undef __WIN32
#endif
#define PortableLockedShm ShmPosix
#include "compat/compat.h"
#include "compat/compat.cpp"
