#include "migration.hpp"
#include "options/options.hpp"

using namespace migrations;
using namespace options;

#include "api/plugin-support.hpp"
#include "compat/library-path.hpp"

struct pt_color_grayscale : migration
{
    bundle b { make_bundle("tracker-pt") };
    enum : int { pt_color_average = 5, pt_color_bt709 = 2, };

    QString unique_date() const override
    {
        return "20220105_00";
    }

    QString name() const override
    {
        return "pt color enum";
    }

    bool should_run() const override
    {
        auto x = b->get_variant("blob-color").toInt();
        return x == pt_color_average;
    }

    void run() override
    {
        b->store_kv("blob-color", QVariant::fromValue((int)pt_color_bt709));
        b->save();
    }
};

OPENTRACK_MIGRATION(pt_color_grayscale)
