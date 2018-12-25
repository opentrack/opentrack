/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#ifndef SHMXX_HEADER_GUARD
#define SHMXX_HEADER_GUARD

#include "export.hpp"
#include "shm.h"

#ifndef SHMXX_TYPE_NAME
#   define SHMXX_TYPE_NAME mem
#endif

class OTR_COMPAT_EXPORT SHMXX_TYPE_NAME final
{
    shm_mem_impl impl;

public:
    SHMXX_TYPE_NAME(const char* shm_name, const char* mutex_name, int map_size);
    ~SHMXX_TYPE_NAME();

    bool success() noexcept;
    void* ptr() noexcept;
    void lock() noexcept;
    void unlock() noexcept;

    SHMXX_TYPE_NAME& operator=(const SHMXX_TYPE_NAME&) = delete;
    SHMXX_TYPE_NAME& operator=(SHMXX_TYPE_NAME&&) noexcept;
};

#endif // SHMXX_HEADER_GUARD
