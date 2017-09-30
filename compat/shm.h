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

#include "export.hpp"

class OTR_COMPAT_EXPORT shm_wrapper final
{
    void* mem;
#if defined(_WIN32)
    HANDLE mutex, mapped_file;
#else
    int fd, size;
#endif

public:
    shm_wrapper(const char *shm_name, const char *mutex_name, int map_size);
    ~shm_wrapper();
    bool lock();
    bool unlock();
    bool success();
    inline void* ptr() { return mem; }
};
