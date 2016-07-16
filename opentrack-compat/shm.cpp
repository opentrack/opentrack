/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "shm.h"

#if defined(_WIN32)

#include <cstring>
#include <stdio.h>

#include <accctrl.h>
#include <aclapi.h>

struct secattr
{
    bool success;
    SECURITY_DESCRIPTOR* pSD;
    SECURITY_ATTRIBUTES attrs;
    PSID pEveryoneSID;
    PACL pACL;

    void cleanup()
    {
        if (pEveryoneSID)
            FreeSid(pEveryoneSID);
        if (pACL)
            LocalFree(pACL);
        if (pSD)
            LocalFree(pSD);
        success = false;
        pSD = nullptr;
        pEveryoneSID = nullptr;
        pACL = nullptr;
    }

    secattr(DWORD perms) : success(true), pSD(nullptr), pEveryoneSID(nullptr), pACL(nullptr)
    {
        SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
        EXPLICIT_ACCESS ea;

        if(!AllocateAndInitializeSid(&SIDAuthWorld, 1,
                         SECURITY_WORLD_RID,
                         0, 0, 0, 0, 0, 0, 0,
                         &pEveryoneSID))
        {
            fprintf(stderr, "AllocateAndInitializeSid: %d\n", (int) GetLastError());
            goto cleanup;
        }

        memset(&ea, 0, sizeof(ea));

        ea.grfAccessPermissions = perms;
        ea.grfAccessMode = SET_ACCESS;
        ea.grfInheritance = NO_INHERITANCE;
        ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
        ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
        ea.Trustee.ptstrName  = (LPTSTR) pEveryoneSID;

        if (SetEntriesInAcl(1, &ea, NULL, &pACL) != ERROR_SUCCESS)
        {
            fprintf(stderr, "SetEntriesInAcl: %d\n", (int) GetLastError());
            goto cleanup;
        }

        pSD = (SECURITY_DESCRIPTOR*) LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
        if (pSD == nullptr)
        {
            fprintf(stderr, "LocalAlloc: %d\n", (int) GetLastError());
            goto cleanup;
        }

        if (!InitializeSecurityDescriptor(pSD,
                    SECURITY_DESCRIPTOR_REVISION))
        {
            fprintf(stderr, "InitializeSecurityDescriptor: %d\n", (int) GetLastError());
            goto cleanup;
        }

        if (!SetSecurityDescriptorDacl(pSD,
                                       TRUE,
                                       pACL,
                                       FALSE))
        {
            fprintf(stderr, "SetSecurityDescriptorDacl: %d\n", (int) GetLastError());
            goto cleanup;
        }

        attrs.bInheritHandle = false;
        attrs.lpSecurityDescriptor = pSD;
        attrs.nLength = sizeof(SECURITY_ATTRIBUTES);

        fflush(stderr);

        return;
cleanup:
        fflush(stderr);
        cleanup();
    }

    ~secattr()
    {
        cleanup();
    }
};

PortableLockedShm::PortableLockedShm(const char* shmName, const char* mutexName, int mapSize)
{
    secattr sa(GENERIC_ALL|SYNCHRONIZE);

    hMutex = CreateMutexA(sa.success ? &sa.attrs : nullptr, false, mutexName);
    if (!hMutex)
    {
        fprintf(stderr, "CreateMutexA: %d\n", (int) GetLastError());
        fflush(stderr);
    }
    hMapFile = CreateFileMappingA(
                 INVALID_HANDLE_VALUE,
                 sa.success ? &sa.attrs : nullptr,
                 PAGE_READWRITE,
                 0,
                 mapSize,
                 shmName);
    if (!hMapFile)
    {
        fprintf(stderr, "CreateFileMappingA: %d\n", (int) GetLastError());
        fflush(stderr);
    }
    mem = MapViewOfFile(hMapFile,
                        FILE_MAP_WRITE,
                        0,
                        0,
                        mapSize);
    if (!mem)
    {
        fprintf(stderr, "MapViewOfFile: %d\n", (int) GetLastError());
        fflush(stderr);
    }
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

#include <limits.h>

#pragma GCC diagnostic ignored "-Wunused-result"
PortableLockedShm::PortableLockedShm(const char *shmName, const char* /*mutexName*/, int mapSize) : size(mapSize)
{
    char filename[PATH_MAX+2] = {0};
    strcpy(filename, "/");
    strcat(filename, shmName);
    fd = shm_open(filename, O_RDWR | O_CREAT, 0600);
    (void) ftruncate(fd, mapSize);
    mem = mmap(NULL, mapSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, (off_t)0);
}

PortableLockedShm::~PortableLockedShm()
{
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

bool PortableLockedShm::success()
{
#ifndef _WIN32
    return mem != (void*) -1;
#else
    return mem != NULL;
#endif
}
