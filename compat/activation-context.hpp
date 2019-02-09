#pragma once

#ifdef _WIN32

#include "export.hpp"

#include <cstdint>
#include <QString>

class OTR_COMPAT_EXPORT activation_context
{
public:
    explicit activation_context(const QString& module_name, int resid);
    ~activation_context();

    explicit operator bool() const { return ok; }

private:
    std::uintptr_t cookie = 0;
    void* handle = (void*)-1;
    bool ok = false;
};

#else
#   error "tried to use win32-only activation context"
#endif
