#ifndef SHM_HEADER_GUARD
#define SHM_HEADER_GUARD

#include "macros1.h"

#ifndef SHM_WIN32_INIT
#   ifdef _WIN32
#       define SHM_WIN32
#   else
#       undef SHM_WIN32
#   endif
#else
#   if SHM_WIN32_INIT
#       define SHM_WIN32
#   else
#       undef SHM_WIN32
#   endif
#endif

#ifndef SHM_TYPE_NAME
#   define SHM_TYPE_NAME shm_mem_impl
#endif

#ifndef SHM_FUN_PREFIX
#   define SHM_FUN_PREFIX shm_mem_impl_
#endif

#ifndef SHM_EXPORT
#   define SHM_EXPORT
#endif

#ifndef __cplusplus
#   define SHM_EXTERN
#   include <stdbool.h>
struct SHM_TYPE_NAME;
typedef struct SHM_TYPE_NAME SHM_TYPE_NAME;
#else
#   define SHM_EXTERN extern "C"
#endif

struct SHM_TYPE_NAME {
    void* mem;
#ifdef SHM_WIN32
    void* mutex;
    void* mapped_file;
#else
    int fd, size;
#endif
};

#define SHM_FUN_NAME(f) PP_CAT(SHM_FUN_PREFIX, f)
#define SHM_FUN(r, f, ...) SHM_EXTERN SHM_EXPORT r SHM_FUN_NAME(f)(SHM_TYPE_NAME* __restrict self, __VA_ARGS__)

SHM_FUN(void, init, const char* shm_name, const char* mutex_name, int map_size);
SHM_FUN(void, free);
SHM_FUN(void, lock);
SHM_FUN(void, unlock);
SHM_FUN(void*,ptr);
SHM_FUN(bool, success);

#ifndef BUILD_SHM
#   undef SHM_FUN
#   undef SHM_FUN_NAME
#endif

#endif // SHM_HEADER_GUARD
