#ifdef _WIN32
#   undef _WIN32
#endif

#define shm_wrapper ShmPosix
#include "compat/shm.h"
#include "compat/shm.cpp"
