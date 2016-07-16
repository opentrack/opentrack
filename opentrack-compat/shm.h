/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#if defined(_WIN32)
#include <windows.h>
#else
#include <stdio.h>
#include <string.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#endif

#ifdef __GNUC__
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wattributes"
#endif

#include "export.hpp"

class OPENTRACK_COMPAT_EXPORT PortableLockedShm {
public:
    PortableLockedShm(const char *shmName, const char *mutexName, int mapSize);
    ~PortableLockedShm();
    void lock();
    void unlock();
    bool success();
    inline void* ptr() { return mem; }
private:
    void* mem;
#if defined(_WIN32)
    HANDLE hMutex, hMapFile;
#else
    int fd, size;
#endif
};

#ifdef __GNUC__
#   pragma GCC diagnostic pop
#endif
