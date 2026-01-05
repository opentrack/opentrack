#pragma once

#include "export.hpp"
#include "compat/macros.h"

#include <optional>

#include <QString>
#include <QSettings>
#include <QRecursiveMutex>

namespace options::globals::detail {

struct saver_;

struct OTR_OPTIONS_EXPORT ini_ctx
{
    std::optional<QSettings> qsettings { std::in_place };
    QString pathname;
    QRecursiveMutex mtx;

    unsigned refcount = 0;
    bool modifiedp = false;

    ini_ctx();
};

struct OTR_OPTIONS_EXPORT saver_ final
{
    ini_ctx& ctx;

    tr_never_inline ~saver_();
    explicit tr_never_inline saver_(ini_ctx& ini);
};

template<typename F>
tr_never_inline
auto with_settings_object_(ini_ctx& ini, F&& fun)
{
    saver_ saver { ini };

    return fun(*ini.qsettings);
}

OTR_OPTIONS_EXPORT ini_ctx& cur_settings();
OTR_OPTIONS_EXPORT ini_ctx& global_settings();

OTR_OPTIONS_EXPORT void mark_ini_modified(bool value = true);

} // ns options::globals::detail

namespace options::globals
{
    OTR_OPTIONS_EXPORT void mark_global_ini_modified(bool value = true);
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
