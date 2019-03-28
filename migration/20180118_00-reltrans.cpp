#include "migration.hpp"
#include "options/options.hpp"

using namespace options;
using namespace migrations;

enum reltrans_state
{
    reltrans_disabled   = 0,
    reltrans_enabled    = 1,
    reltrans_non_center = 2,
};

static const char* old_name = "compensate-translation";
static const char* new_name = "relative-translation-mode";

struct reltrans_enum : migration
{
    QString unique_date() const override
    {
        return "20180118_00";
    }

    QString name() const override
    {
        return "reltrans modes";
    }

    bool should_run() const override
    {
        auto b = make_bundle("opentrack-ui");
        return b->contains(old_name) && !b->contains(new_name);
    }

    void run() override
    {
        auto b = make_bundle("opentrack-ui");
        bool value = b->get_variant(old_name).value<bool>();
        b->store_kv(new_name, int(value ? reltrans_enabled : reltrans_disabled));
        b->save();
    }
};

OPENTRACK_MIGRATION(reltrans_enum)
