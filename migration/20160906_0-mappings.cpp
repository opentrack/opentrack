#include "migration.hpp"

#include <QDebug>

using namespace migrations;

struct foo : migration
{
    const QString& unique_date() const override { qDebug() << "foo"; static QString ret(""); return ret; }
    bool should_run() const override { qDebug() << "bar"; return false; }
    bool run() override { return false; }
};

OPENTRACK_MIGRATION(foo);
