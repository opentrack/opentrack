#include "options.hpp"

namespace options
{

namespace detail
{
OPENTRACK_COMPAT_EXPORT opt_singleton& singleton()
{
    static opt_singleton ret;
    return ret;
}

}

group::group(const QString& name) : name(name)
{
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

void group::save()
{
    auto s = ini_file();
    s->beginGroup(name);
    for (auto& i : kvs)
        s->setValue(i.first, i.second);
    s->endGroup();
    s->sync();
}

void group::put(const QString &s, const QVariant &d)
{
    kvs[s] = d;
}

bool group::contains(const QString &s)
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
    QSettings settings(OPENTRACK_ORG);
    return dir + "/" + settings.value(OPENTRACK_CONFIG_FILENAME_KEY, OPENTRACK_DEFAULT_CONFIG).toString();
}

const QStringList group::ini_list()
{
    const auto dirname = ini_directory();
    if (dirname == "")
        return QStringList();
    QDir settings_dir(dirname);
    return settings_dir.entryList( QStringList { "*.ini" } , QDir::Files, QDir::Name );
}

const mem<QSettings> group::ini_file()
{
    const auto pathname = ini_pathname();
    if (pathname != "")
        return std::make_shared<QSettings>(ini_pathname(), QSettings::IniFormat);
    return std::make_shared<QSettings>();
}

impl_bundle::impl_bundle(const QString &group_name)
    :
      mtx(QMutex::Recursive),
      group_name(group_name),
      saved(group_name),
      transient(saved),
      modified(false)
{}

void impl_bundle::reload()
{
    {
        QMutexLocker l(&mtx);
        saved = group(group_name);
        transient = saved;
        modified = false;
    }
    emit reloading();
}

void impl_bundle::store_kv(const QString &name, const QVariant &datum)
{
    QMutexLocker l(&mtx);

    auto old = transient.get<QVariant>(name);
    if (!transient.contains(name) || datum != old)
    {
        modified = true;
        transient.put(name, datum);
    }
}

bool impl_bundle::contains(const QString &name)
{
    QMutexLocker l(&mtx);
    return transient.contains(name);
}

void impl_bundle::save()
{
    {
        QMutexLocker l(&mtx);
        modified = false;
        saved = transient;
        transient.save();
    }
    emit saving();
}

bool impl_bundle::modifiedp()
{
    QMutexLocker l(&mtx);
    return modified;
}

namespace detail
{

pbundle opt_singleton::bundle(const opt_singleton::k &key)
{
    QMutexLocker l(&implsgl_mtx);

    if (implsgl_data.count(key) != 0)
    {
        auto shared = std::get<1>(implsgl_data[key]).lock();
        if (shared != nullptr)
            return shared;
    }
    
    qDebug() << "bundle +" << key;

    auto shr = std::make_shared<v>(key);
    implsgl_data[key] = tt(cnt(1), shr);
    return shr;
}

void opt_singleton::bundle_decf(const opt_singleton::k &key)
{
    QMutexLocker l(&implsgl_mtx);

    if (--std::get<0>(implsgl_data[key]) == 0)
        implsgl_data.erase(key);
}

opt_singleton::opt_singleton() : implsgl_mtx(QMutex::Recursive) {}

}

opt_bundle::opt_bundle(const QString &group_name)
    : impl_bundle(group_name)
{
}

opt_bundle::~opt_bundle()
{
    qDebug() << "bundle -" << group_name;
    detail::singleton().bundle_decf(group_name);
}

base_value::base_value(pbundle b, const QString &name) : b(b), self_name(name) {}

opts::~opts()
{
    b->reload();
}

opts::opts(const QString &name) : b(bundle(name)) {}

}
