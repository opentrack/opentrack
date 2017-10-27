/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "shm.h"

#if defined(_WIN32)

#if !defined __WINE__
#   include <QDebug>
#endif

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

        return;
cleanup:
        cleanup();
    }

    ~secattr()
    {
        cleanup();
    }
};

shm_wrapper::shm_wrapper(const char* shm_name, const char* mutex_name, int map_size)
{
    secattr sa(GENERIC_ALL|SYNCHRONIZE);

    if (mutex_name == nullptr)
        mutex = nullptr;
    else
    {
        mutex = CreateMutexA(sa.success ? &sa.attrs : nullptr, false, mutex_name);

        if (!mutex)
        {
    #if !defined __WINE__
            qDebug() << "CreateMutexA:" << (int) GetLastError();
    #endif
            return;
        }
    }

    mapped_file = CreateFileMappingA(
                 INVALID_HANDLE_VALUE,
                 sa.success ? &sa.attrs : nullptr,
                 PAGE_READWRITE,
                 0,
                 map_size,
                 shm_name);

    if (!mapped_file)
    {
#if !defined __WINE__
        qDebug() << "CreateFileMappingA:", (int) GetLastError();
#endif

        return;
    }

    mem = MapViewOfFile(mapped_file,
                        FILE_MAP_WRITE,
                        0,
                        0,
                        map_size);

    if (!mem)
    {
#if !defined __WINE__
        qDebug() << "MapViewOfFile:" << (int) GetLastError();
#endif
    }
}

shm_wrapper::~shm_wrapper()
{
    if(!UnmapViewOfFile(mem))
        goto fail;

    if (!CloseHandle(mapped_file))
        goto fail;

    if (mutex && !CloseHandle(mutex))
        goto fail;

    return;

fail:
    (void)0;
#if !defined __WINE__
    qDebug() << "failed to close mapping";
#endif
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
    mem = mmap(NULL, map_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, (off_t)0);
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
    return mem != NULL;
#endif
}
