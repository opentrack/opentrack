#include "migration.hpp"
#include "options/options.hpp"

using namespace options;
using namespace options::globals;
using namespace migrations;

static constexpr auto OLD_RECORD_SEPARATOR  = QChar('|');
static constexpr auto OLD_UNIT_SEPARATOR    = QChar(':');

static constexpr auto NEW_RECORD_SEPARATOR  = QChar(0x1e);
static constexpr auto NEW_UNIT_SEPARATOR    = QChar(0x1f);

static const QString KEY_NAME = "executable-list";

struct process_detector_record_separator : migration
{
    QString unique_date() const override
    {
        return "20180102_00";
    }

    QString name() const override
    {
        return "process detector record separator";
    }

    bool should_run() const override
    {
        return with_global_settings_object([](const QSettings& s)
        {
            const QString old_value = s.value(KEY_NAME).toString();
            return old_value.contains(OLD_RECORD_SEPARATOR);
        });
    }

    void run() override
    {
        return with_global_settings_object([](QSettings& s)
        {
            QString value = s.value(KEY_NAME).toString();
            value.replace(OLD_UNIT_SEPARATOR,   NEW_UNIT_SEPARATOR);
            value.replace(OLD_RECORD_SEPARATOR, NEW_RECORD_SEPARATOR);
            s.setValue(KEY_NAME, value);
            mark_global_ini_modified();
        });
    }
};

OPENTRACK_MIGRATION(process_detector_record_separator)
