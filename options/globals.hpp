#pragma once

#include "export.hpp"
#include "compat/macros.hpp"

#include <optional>

#include <QString>
#include <QSettings>
#include <QMutex>

namespace options::globals::detail {

struct ini_ctx;
struct saver_;

OTR_OPTIONS_EXPORT ini_ctx& cur_settings();
OTR_OPTIONS_EXPORT ini_ctx& global_settings();
OTR_OPTIONS_EXPORT bool is_portable_installation();

struct ini_ctx
{
    std::optional<QSettings> qsettings { std::in_place };
    int refcount = 0;
    bool modifiedp = false;
    QMutex mtx { QMutex::Recursive };
    QString pathname;
};

struct OTR_OPTIONS_EXPORT saver_ final
{
    ini_ctx& ctx;

    cc_noinline ~saver_();
    explicit cc_noinline saver_(ini_ctx& ini);
};

template<typename F>
cc_noinline
auto with_settings_object_(ini_ctx& ini, F&& fun)
{
    saver_ saver { ini };

    return fun(*ini.qsettings);
}

} // ns options::globals::detail

namespace options::globals
{
    OTR_OPTIONS_EXPORT void mark_ini_modified();
    OTR_OPTIONS_EXPORT QString ini_directory();
    OTR_OPTIONS_EXPORT QString ini_filename();
    OTR_OPTIONS_EXPORT QString ini_pathname();
    OTR_OPTIONS_EXPORT QString ini_combine(const QString& filename);
    OTR_OPTIONS_EXPORT QStringList ini_list();

    template<typename F>
    auto with_settings_object(F&& fun)
    {
        using namespace detail;
        return with_settings_object_(cur_settings(), fun);
    }

    template<typename F>
    auto with_global_settings_object(F&& fun)
    {
        using namespace detail;
        return with_settings_object_(global_settings(), fun);
    }
} // ns options::globals
