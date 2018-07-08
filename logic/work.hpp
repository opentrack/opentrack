/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "main-settings.hpp"
#include "api/plugin-support.hpp"
#include "pipeline.hpp"
#include "shortcuts.h"
#include "export.hpp"
#include "tracklogger.hpp"
#include "logic/runtime-libraries.hpp"
#include "api/plugin-support.hpp"
#include "compat/tr.hpp"

#include <QObject>
#include <QFrame>
#include <memory>
#include <vector>
#include <tuple>
#include <functional>

class OTR_LOGIC_EXPORT Work final : public TR
{
    Q_OBJECT

    using dylibptr = std::shared_ptr<dylib>;

    static std::unique_ptr<TrackLogger> make_logger(main_settings &s);
    static QString browse_datalogging_file(main_settings &s);

public:
    using fn_t = std::function<void(bool)>;
    using key_tuple = std::tuple<key_opts&, fn_t, bool>;
    main_settings s; // tracker needs settings, so settings must come before it
    runtime_libraries libs; // idem
    std::unique_ptr<TrackLogger> logger; // must come before tracker, since tracker depends on it
    pipeline pipeline_;
    Shortcuts sc;
    std::vector<key_tuple> keys;

    Work(Mappings& m, event_handler& ev, QFrame* frame,
         const dylibptr& tracker, const dylibptr& filter, const dylibptr& proto);
    void reload_shortcuts();
    bool is_ok() const;
};
