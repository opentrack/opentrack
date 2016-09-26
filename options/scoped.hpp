#pragma once

#include "bundle.hpp"
#include <QString>

#include "export.hpp"

#include <atomic>

namespace options {

struct OPENTRACK_OPTIONS_EXPORT opts
{
    bundle b;
    opts(const QString& name);
    opts& operator=(const opts&) = delete;
    opts(const opts&) = delete;
    ~opts();

    // XXX hack: the flag shouldn't be here as action at distance -sh 20160926
    static void set_teardown_flag(bool value);
private:
    static std::atomic_bool teardown_flag;
    static bool is_tracker_teardown();
    static void ensure_thread();
};

}
