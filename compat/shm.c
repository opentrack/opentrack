#define BUILD_SHM
#include "shm.h"

#ifdef SHM_WIN32

#include <windows.h>

SHM_FUN(void, init, const char* shm_name, const char* mutex_name, int map_size)
{
    if (mutex_name != NULL)
    {
        self->mutex = CreateMutexA(NULL, false, mutex_name);

        if (!self->mutex)
            goto fail;
    }

    self->mapped_file = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        (unsigned)map_size,
        shm_name);

    if (!self->mapped_file)
        goto fail;

    self->mem = MapViewOfFile(self->mapped_file,
                              FILE_MAP_WRITE,
                              0,
                              0,
                              (unsigned) map_size);

    if (!self->mem)
        goto fail;

    return;

fail:
    SHM_FUN_NAME(free)(self);
}

SHM_FUN(void, free)
{
    if (self->mem)
        (void) UnmapViewOfFile(self->mem);

    if (self->mapped_file)
        (void) CloseHandle(self->mapped_file);

    if (self->mutex)
        (void) CloseHandle(self->mutex);

    self->mem = NULL;
    self->mapped_file = NULL;
    self->mutex = NULL;
}

SHM_FUN(void, lock)
{
    if (self->mutex)
        (void)(WaitForSingleObject(self->mutex, INFINITE) == WAIT_OBJECT_0);
}

SHM_FUN(void, unlock)
{
    (void) ReleaseMutex(self->mutex);
}

SHM_FUN(bool, success)
{
    return self->mem != NULL;
}

#else

#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <alloca.h>

//#pragma GCC diagnostic ignored "-Wunused-result"

SHM_FUN(void, init, const char *shm_name, const char* mutex_name, int map_size)
{
    char* filename = alloca(strlen(shm_name)+2);
    (void)mutex_name;

    self->mem = (void*)-1;
    self->fd = -1;
    self->size = 0;

    if (map_size <= 0)
        goto fail;

    self->size = map_size;
    strcpy(filename, "/");
    strcat(filename, shm_name);
    self->fd = shm_open(filename, O_RDWR | O_CREAT, 0600);
    (void)ftruncate(self->fd, (off_t)map_size);
    self->mem = mmap(NULL, (size_t)map_size, PROT_READ|PROT_WRITE, MAP_SHARED, self->fd, (off_t)0);

    if (self->mem == (void*)-1)
        goto fail;

    return;

fail:
    SHM_FUN_NAME(free)(self);
}

SHM_FUN(void, free)
{
    if (self->mem != (void*)-1)
        (void)munmap(self->mem, self->size);
    if (self->fd != -1)
        (void)close(self->fd);

    self->mem = (void*)-1;
    self->fd = -1;
    self->size = 0;
}

SHM_FUN(void, lock)
{
    return flock(self->fd, LOCK_EX) == 0;
}

SHM_FUN(void, unlock)
{
    return flock(self->fd, LOCK_UN) == 0;
}

SHM_FUN(bool, success)
{
    return self->mem != (void*) -1;
}

#endif

SHM_FUN(void*, ptr)
{
    return self->mem;
}
