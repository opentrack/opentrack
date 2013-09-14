/* Copyright (c) 2013 Stanisław Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#define IN_FTNOIR_COMPAT
#include "compat.h"

#if defined(_WIN32)

PortableLockedShm::PortableLockedShm(const char* shmName, const char* mutexName, int mapSize)
{
    hMutex = CreateMutexA(NULL, false, mutexName);
    hMapFile = CreateFileMappingA(
                 INVALID_HANDLE_VALUE,
                 NULL,
                 PAGE_READWRITE,
                 0,
                 mapSize,
                 shmName);
    mem = MapViewOfFile(hMapFile,
                        FILE_MAP_WRITE,
                        0,
                        0,
                        mapSize);
}

PortableLockedShm::~PortableLockedShm()
{
    UnmapViewOfFile(mem);
    CloseHandle(hMapFile);
    CloseHandle(hMutex);
}

void PortableLockedShm::lock()
{
    (void) WaitForSingleObject(hMutex, INFINITE);
}

void PortableLockedShm::unlock()
{
    (void) ReleaseMutex(hMutex);
}

#else
PortableLockedShm::PortableLockedShm(const char *shmName, const char *mutexName, int mapSize) : size(mapSize)
{
    char shm_filename[NAME_MAX];
    shm_filename[0] = '/';
    strncpy(shm_filename+1, shmName, NAME_MAX-2);
    sprintf(shm_filename + strlen(shm_filename), "%ld\n", (long) getuid());
    shm_filename[NAME_MAX-1] = '\0';

    //(void) shm_unlink(shm_filename);

    fd = shm_open(shm_filename, O_RDWR | O_CREAT, 0600);
    if (ftruncate(fd, mapSize) == 0)
        mem = mmap(NULL, mapSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, (off_t)0);
    else
        mem = (void*) -1;
}

PortableLockedShm::~PortableLockedShm()
{
    //(void) shm_unlink(shm_filename);

    (void) munmap(mem, size);
    (void) close(fd);
}

void PortableLockedShm::lock()
{
    flock(fd, LOCK_EX);
}

void PortableLockedShm::unlock()
{
    flock(fd, LOCK_UN);
}



#endif
