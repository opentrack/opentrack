/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#define BUILD_SHM
#include "shm.hpp"

SHMXX_TYPE_NAME& SHMXX_TYPE_NAME::operator=(SHMXX_TYPE_NAME&&) noexcept = default;

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
SHMXX_TYPE_NAME::SHMXX_TYPE_NAME(const char* shm_name, const char* mutex_name, int map_size)
{
    shm_mem_impl_init(&impl, shm_name, mutex_name, map_size);
}

SHMXX_TYPE_NAME::~SHMXX_TYPE_NAME()
{
    shm_mem_impl_free(&impl);
}

bool SHMXX_TYPE_NAME::success() noexcept { return shm_mem_impl_success(&impl); }
void SHMXX_TYPE_NAME::lock() noexcept { shm_mem_impl_lock(&impl); }
void SHMXX_TYPE_NAME::unlock() noexcept { shm_mem_impl_unlock(&impl); }
void* SHMXX_TYPE_NAME::ptr() noexcept { return shm_mem_impl_ptr(&impl); }
