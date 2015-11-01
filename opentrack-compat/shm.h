/* Copyright (c) 2013 Stanisław Halik <sthalik@misaki.pl>

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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"

#ifdef __GNUC__
#   define COMPAT_GNUC_VISIBILITY __attribute__ ((visibility ("default")))
#else
#   define COMPAT_GNUC_VISIBILITY
#endif

#ifdef BUILD_compat
#   ifdef _WIN32
#       define COMPAT_WIN32_EXPORT __declspec(dllexport)
#   else
#       define COMPAT_WIN32_EXPORT
#   endif
#else
#   ifdef _WIN32
#       define COMPAT_WIN32_EXPORT __declspec(dllimport)
#   else
#       define COMPAT_WIN32_EXPORT
#   endif
#endif

#define COMPAT_EXPORT COMPAT_WIN32_EXPORT COMPAT_GNUC_VISIBILITY

class COMPAT_EXPORT PortableLockedShm {
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

#pragma GCC diagnostic pop
