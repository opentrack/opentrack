/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "options/options.hpp"
#include "api/plugin-support.hpp"
#include "main-settings.hpp"
#include "mappings.hpp"
#include "work.hpp"
#include "export.hpp"

#include <memory>
#include <QString>

struct OTR_LOGIC_EXPORT State
{
    using dylib_ptr = Modules::dylib_ptr;
    using dylib_list = Modules::dylib_list;

    explicit State(const QString& library_path);
    static std::tuple<dylib_ptr, int> module_by_name(const QString& name, dylib_list& list);

    dylib_ptr current_tracker();
    dylib_ptr current_protocol();
    dylib_ptr current_filter();

    Modules modules;
    main_settings s;
    module_settings m;
    Mappings pose;
    std::shared_ptr<Work> work;
    QString library_path;
};
