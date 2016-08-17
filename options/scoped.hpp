#pragma once

#include "bundle.hpp"
#include <QString>

#include "export.hpp"

namespace options {

struct OPENTRACK_OPTIONS_EXPORT opts
{
    bundle b;
    opts(const QString& name);
    opts& operator=(const opts&) = delete;
    opts(const opts&) = delete;
    ~opts();
};

}
