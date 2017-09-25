#pragma once

#include "bundle.hpp"
#include <QString>

#include "export.hpp"

#include <atomic>

namespace options {

struct OTR_OPTIONS_EXPORT opts
{
    bundle b;
    opts(const QString& name);
    opts& operator=(const opts&) = delete;
    opts(const opts&) = delete;
    virtual ~opts();

    // XXX hack: the flag shouldn't be here as action at distance -sh 20160926
    static void set_teardown_flag(bool value);
    static bool is_tracker_teardown();
private:
    static std::atomic_bool teardown_flag;
    static void ensure_thread();
};

}
