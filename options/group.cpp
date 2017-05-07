/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "group.hpp"
#include "defs.hpp"

#include "compat/timer.hpp"

#include <cmath>

#include <QStandardPaths>
#include <QDir>
#include <QDebug>

namespace options {

group::group(const QString& name) : name(name)
{
    if (name == "")
        return;

    with_settings_object([&](QSettings& conf) {
        conf.beginGroup(name);
        for (auto& k_ : conf.childKeys())
        {
            auto tmp = k_.toUtf8();
            QString k(tmp);
            kvs[k] = conf.value(k_);
        }
        conf.endGroup();
    });
}

void group::save() const
{
    if (name == "")
        return;

    with_settings_object([&](QSettings& s) {
        s.beginGroup(name);
        for (auto& i : kvs)
            s.setValue(i.first, i.second);
        s.endGroup();

        mark_ini_modified();
    });
}

void group::put(const QString &s, const QVariant &d)
{
    kvs[s] = d;
}

bool group::contains(const QString &s) const
{
    return kvs.find(s) != kvs.cend();
}

QString group::ini_directory()
{
    const auto dirs = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    if (dirs.size() == 0)
        return "";
    if (QDir(dirs[0]).mkpath(OPENTRACK_ORG))
        return dirs[0] + "/" OPENTRACK_ORG;
    return "";
}

QString group::ini_filename()
{
    QSettings settings(OPENTRACK_ORG);
    const QString ret = settings.value(OPENTRACK_CONFIG_FILENAME_KEY, OPENTRACK_DEFAULT_CONFIG).toString();
    if (ret.size() == 0)
        return OPENTRACK_DEFAULT_CONFIG;
    return ret;
}

QString group::ini_pathname()
{
    const auto dir = ini_directory();
    if (dir == "")
        return "";
    return dir + "/" + ini_filename();
}

QString group::ini_combine(const QString& filename)
{
    return ini_directory() + QStringLiteral("/") + filename;
}

QStringList group::ini_list()
{
    const auto dirname = ini_directory();
    if (dirname == "")
        return QStringList();
    QDir settings_dir(dirname);
    QStringList list = settings_dir.entryList( QStringList { "*.ini" } , QDir::Files, QDir::Name );
    std::sort(list.begin(), list.end());
    return list;
}

void group::mark_ini_modified()
{
    QMutexLocker l(&cur_ini_mtx);
    ini_modifiedp = true;
}

QString group::cur_ini_pathname;
std::shared_ptr<QSettings> group::cur_ini;
QMutex group::cur_ini_mtx(QMutex::Recursive);
int group::ini_refcount = 0;
bool group::ini_modifiedp = false;

std::shared_ptr<QSettings> group::cur_settings_object()
{
    const QString pathname = ini_pathname();

    if (pathname.isEmpty())
        return std::make_shared<QSettings>();

    QMutexLocker l(&cur_ini_mtx);

    if (pathname != cur_ini_pathname)
    {
        cur_ini = std::make_shared<QSettings>(pathname, QSettings::IniFormat);
        cur_ini_pathname = pathname;
    }

    return cur_ini;
}

group::saver_::~saver_()
{
    if (--ini_refcount == 0 && ini_modifiedp)
    {
        ini_modifiedp = false;
        static Timer t;
        const double tm = t.elapsed_seconds();
        qDebug() << QStringLiteral("%1.%2").arg(int(tm)).arg(int(std::fmod(tm, 1.)*10)).toLatin1().data()
                 << "saving .ini file" << cur_ini_pathname;
        s.sync();
    }
}

group::saver_::saver_(QSettings& s, QMutex& mtx) : s(s), mtx(mtx), lck(&mtx)
{
    ini_refcount++;
}

} // ns options
