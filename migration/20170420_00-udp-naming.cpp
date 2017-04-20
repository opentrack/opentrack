/* Copyright (c) 2017, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "migration.hpp"
#include "options/options.hpp"
#include "compat/util.hpp"
#include "logic/main-settings.hpp"

using namespace migrations;
using namespace options;

static constexpr const char* old_tracker_name = "UDP sender";
static constexpr const char* old_proto_name = "UDP Tracker";

static constexpr const char* new_tracker_name = "UDP over network";
static constexpr const char* new_proto_name = "UDP over network";

struct rename_udp_stuff : migration
{
    bool should_run() const override
    {
        module_settings s;
        return s.protocol_dll == old_proto_name || s.tracker_dll == old_tracker_name;
    }

    void run() override
    {
        module_settings s;

        if (s.protocol_dll == old_proto_name)
            s.protocol_dll = new_proto_name;

        if (s.tracker_dll == old_tracker_name)
            s.tracker_dll = new_tracker_name;

        s.b->save();
    }

    QString unique_date() const override
    {
        return "20170420_00";
    }

    QString name() const override
    {
        return "rename confusing UDP tracker/proto names";
    }
};

OPENTRACK_MIGRATION(rename_udp_stuff);
