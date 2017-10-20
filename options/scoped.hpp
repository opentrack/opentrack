#pragma once

#include "bundle.hpp"
#include <QString>

#include "export.hpp"

#include <atomic>

namespace options {

struct OTR_OPTIONS_EXPORT with_tracker_teardown final
{
    with_tracker_teardown();
    ~with_tracker_teardown();

private:
    bool old_value;
};

struct OTR_OPTIONS_EXPORT opts
{
    bundle b;
    opts(const QString& name);
    opts& operator=(const opts&) = delete;
    opts(const opts&) = delete;
    virtual ~opts();
};

}
