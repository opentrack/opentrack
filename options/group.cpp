#include "group.hpp"
#include "defs.hpp"
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

namespace options {

group::group(const QString& name) : name(name)
{
    if (name == "")
        return;

    auto conf = ini_file();
    conf->beginGroup(name);
    for (auto& k_ : conf->childKeys())
    {
        auto tmp = k_.toUtf8();
        QString k(tmp);
        kvs[k] = conf->value(k_);
    }
    conf->endGroup();
}

void group::save() const
{
    save_deferred(*ini_file());
}

void group::save_deferred(QSettings& s) const
{
    if (name == "")
        return;

    s.beginGroup(name);
    for (auto& i : kvs)
        s.setValue(i.first, i.second);
    s.endGroup();
}

void group::put(const QString &s, const QVariant &d)
{
    kvs[s] = d;
}

bool group::contains(const QString &s) const
{
    return kvs.count(s) != 0;
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
    return settings.value(OPENTRACK_CONFIG_FILENAME_KEY, OPENTRACK_DEFAULT_CONFIG).toString();
}

QString group::ini_pathname()
{
    const auto dir = ini_directory();
    if (dir == "")
        return "";
    return dir + "/" + ini_filename();
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

std::shared_ptr<QSettings> group::ini_file()
{
    const auto pathname = ini_pathname();
    if (pathname != "")
        return std::make_shared<QSettings>(ini_pathname(), QSettings::IniFormat);
    return std::make_shared<QSettings>();
}

}
