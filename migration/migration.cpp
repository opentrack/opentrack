#include "migration.hpp"

#include "options/options.hpp"
#include "compat/util.hpp"

#include <QString>
#include <QSettings>
#include <QDebug>

#include <memory>

namespace migrations {

namespace detail {

void migrator::register_migration(migration* m)
{
    migrations().push_back(m);
}

migrator::vec& migrator::migrations()
{
    static vec ret;
    return ret;
}

QString migrator::last_migration_time()
{
    QString ret;

    std::shared_ptr<QSettings> s(options::group::ini_file());

    s->beginGroup("migrations");
    ret = s->value("last-migration-at", "19700101_00").toString();
    s->endGroup();

    return ret;
}

QString migrator::time_after_migrations()
{
    const vec list = sorted_migrations();

    if (list.size() == 0u)
        return QStringLiteral("19700101_00");

    QString ret = list[list.size() - 1]->unique_date();
    ret += QStringLiteral("~");

    return ret;
}

void migrator::set_last_migration_time(const QString& val)
{
    std::shared_ptr<QSettings> s(options::group::ini_file());

    s->beginGroup("migrations");
    s->setValue("last-migration-at", val);
    s->endGroup();
}

migrator::vec migrator::sorted_migrations()
{
    vec list(migrations());
    std::sort(list.begin(), list.end(), [](const mm x, const mm y) { return x->unique_date() < y->unique_date(); });
    return list;
}

std::vector<QString> migrator::run()
{
    vec migrations = sorted_migrations();
    vstr done;

    const QString last_migration = last_migration_time();

    for (migration* m_ : migrations)
    {
        migration& m(*m_);

        const QString date = m.unique_date();

        if (date <= last_migration)
            continue;

        if (m.should_run())
        {
            m.run();
            done.push_back(m.name());
        }
    }

    mark_config_as_not_needing_migration();

    if (done.size())
    {
        for (const QString& name : done)
            qDebug() << "--" << name;
    }

    return done;
}

}

migration::migration() {}
migration::~migration() {}

} // ns

std::vector<QString> run_migrations()
{
    return migrations::detail::migrator::run();
}

void mark_config_as_not_needing_migration()
{
    using m = migrations::detail::migrator;

    m::mark_config_as_not_needing_migration();
}

void migrations::detail::migrator::mark_config_as_not_needing_migration()
{
    set_last_migration_time(time_after_migrations());
}
