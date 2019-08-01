/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "shm.h"

#if defined _WIN32

#include <cstring>
#include <cstdio>

#include <accctrl.h>
#include <aclapi.h>

#ifdef QT_CORE_LIB
#   include <QDebug>
#   define warn(str, ...) (qDebug() << "shm:" str ": " << __VA_ARGS__)
#else
#   define warn(str, ...) (void)0
#endif

shm_wrapper::shm_wrapper(const char* shm_name, const char* mutex_name, int map_size)
{
    if (mutex_name == nullptr)
        mutex = nullptr;
    else
    {
        mutex = CreateMutexA(nullptr, false, mutex_name);

        if (!mutex)
        {
            warn("CreateMutexA", (int) GetLastError());
            return;
        }
    }

    mapped_file = CreateFileMappingA(
                 INVALID_HANDLE_VALUE,
                 nullptr,
                 PAGE_READWRITE,
                 0,
                 map_size,
                 shm_name);

    if (!mapped_file)
    {
        warn("CreateFileMappingA", (int) GetLastError());

        return;
    }

    mem = MapViewOfFile(mapped_file,
                        FILE_MAP_WRITE,
                        0,
                        0,
                        map_size);

    if (!mem)
        warn("MapViewOfFile:", (int) GetLastError());
}

shm_wrapper::~shm_wrapper()
{
    if (mem && !UnmapViewOfFile(mem))
        goto fail;

    if (mapped_file && !CloseHandle(mapped_file))
        goto fail;

    if (mutex && !CloseHandle(mutex))
        goto fail;

    return;

fail:
    warn("failed to close mapping", (int) GetLastError());
}

bool shm_wrapper::lock()
{
    if (mutex)
        return WaitForSingleObject(mutex, INFINITE) == WAIT_OBJECT_0;
    else
        return false;
}

bool shm_wrapper::unlock()
{
    if (mutex)
        return ReleaseMutex(mutex);
    else
        return false;
}
#else

#include <limits.h>

#pragma GCC diagnostic ignored "-Wunused-result"
shm_wrapper::shm_wrapper(const char *shm_name, const char* /*mutex_name*/, int map_size) : size(map_size)
{
    char filename[PATH_MAX+2] {};
    strcpy(filename, "/");
    strcat(filename, shm_name);
    fd = shm_open(filename, O_RDWR | O_CREAT, 0600);
    (void) ftruncate(fd, map_size);
    mem = mmap(nullptr, map_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, (off_t)0);
}

shm_wrapper::~shm_wrapper()
{
    (void) munmap(mem, size);
    (void) close(fd);
}

bool shm_wrapper::lock()
{
    return flock(fd, LOCK_EX) == 0;
}

bool shm_wrapper::unlock()
{
    return flock(fd, LOCK_UN) == 0;
}
#endif

bool shm_wrapper::success()
{
#ifndef _WIN32
    return mem != (void*) -1;
#else
    return mem != nullptr;
#endif
}

