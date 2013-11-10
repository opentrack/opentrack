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

#if !defined(OPENTRACK_COMPAT_BUNDLED)
#   if defined(IN_FTNOIR_COMPAT) && defined(_WIN32)
#       define COMPAT_EXPORT __declspec(dllexport)
#   elif defined(_WIN32)
#       define COMPAT_EXPORT __declspec(dllimport)
#   else
#       define COMPAT_EXPORT __attribute__ ((visibility ("default")))
#   endif
#else
#   define COMPAT_EXPORT
#endif

class COMPAT_EXPORT PortableLockedShm {
public:
    PortableLockedShm(const char *shmName, const char *mutexName, int mapSize);
    ~PortableLockedShm();
    void lock();
    void unlock();
    bool success();
    void* mem;
private:
#if defined(_WIN32)
    HANDLE hMutex, hMapFile;
#else
    int fd, size;
    //char shm_filename[NAME_MAX];
#endif
};
