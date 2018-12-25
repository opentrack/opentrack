#undef _WIN32

#define SHM_TYPE_NAME shm_impl_unix
#define SHM_FUN_PREFIX shm_impl_unix_
#define SHMXX_TYPE_NAME mem_unix
#include "compat/shm.cpp"
