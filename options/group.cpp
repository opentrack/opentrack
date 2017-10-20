/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "group.hpp"
#include "defs.hpp"

#include "compat/timer.hpp"
#include "opentrack-library-path.h"

#include <cmath>

#include <QFile>
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
            QVariant val = conf.value(k_);
            if (val.type() != QVariant::Invalid)
                kvs[k] = std::move(val);
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
    const auto it = kvs.find(s);
    return it != kvs.cend() && it->second != QVariant::Invalid;
}

bool group::is_portable_installation()
{
#if defined _WIN32
    if (QFile::exists(OPENTRACK_BASE_PATH + "/portable.txt"))
        return true;
#endif
    return false;
}

QString group::ini_directory()
{

    QString dir;

    if (is_portable_installation())
    {
        dir = OPENTRACK_BASE_PATH;

        static const QString subdir = "ini";

        if (!QDir(dir).mkpath(subdir))
            return QString();

        return dir + '/' + subdir;
    }
    else
    {
        dir = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).value(0, QString());
        if (dir.isEmpty())
            return QString();
        if (!QDir(dir).mkpath(OPENTRACK_ORG))
            return QString();

        dir += '/';
        dir += OPENTRACK_ORG;
    }

    return dir;
}

QString group::ini_filename()
{
    return with_global_settings_object([&](QSettings& settings) {
        const QString ret = settings.value(OPENTRACK_CONFIG_FILENAME_KEY, OPENTRACK_DEFAULT_CONFIG).toString();
        if (ret.size() == 0)
            return QStringLiteral(OPENTRACK_DEFAULT_CONFIG);
        return ret;
    });
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

std::shared_ptr<QSettings> group::cur_global_ini;
QMutex group::global_ini_mtx(QMutex::Recursive);
int group::global_ini_refcount = 0;
bool group::global_ini_modifiedp = false;

std::shared_ptr<QSettings> group::cur_settings_object()
{
    const QString pathname = ini_pathname();

    if (pathname.isEmpty())
        return std::make_shared<QSettings>();

    if (pathname != cur_ini_pathname)
    {
        cur_ini = std::make_shared<QSettings>(pathname, QSettings::IniFormat);
        cur_ini_pathname = pathname;
    }

    return cur_ini;
}

std::shared_ptr<QSettings> group::cur_global_settings_object()
{
    if (cur_global_ini)
        return cur_global_ini;

    if (!is_portable_installation())
        cur_global_ini = std::make_shared<QSettings>(OPENTRACK_ORG);
    else
    {
        static const QString pathname = OPENTRACK_BASE_PATH + QStringLiteral("/globals.ini");
        cur_global_ini = std::make_shared<QSettings>(pathname, QSettings::IniFormat);
    }

    return cur_global_ini;
}

never_inline
group::saver_::~saver_()
{
    if (--refcount == 0 && modifiedp)
    {
        modifiedp = false;
        s.sync();
        if (s.status() != QSettings::NoError)
            qDebug() << "error with .ini file" << s.fileName() << s.status();
    }
}

never_inline
group::saver_::saver_(QSettings& s, int& refcount, bool& modifiedp) :
    s(s), refcount(refcount), modifiedp(modifiedp)
{
    refcount++;
}

} // ns options
