#include "options.hpp"

#include <cmath>

namespace options
{

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

void group::save() const
{
    auto s = ini_file();
    s->beginGroup(name);
    for (auto& i : kvs)
        s->setValue(i.first, i.second);
    s->endGroup();
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

bool group::operator==(const group& other) const
{
    for (const auto& kv : kvs)
    {
        const QVariant val = other.get<QVariant>(kv.first);
        if (!other.contains(kv.first) || kv.second != val)
        {
            qDebug() << "bundle" << name << "modified" << "key" << kv.first << "-" << val << "<>" << kv.second;
            return false;
        }
    }

    for (const auto& kv : other.kvs)
    {
        const QVariant val = get<QVariant>(kv.first);
        if (!contains(kv.first) || kv.second != val)
        {
            qDebug() << "bundle" << name << "modified" << "key" << kv.first << "-" << kv.second << "<>" << val;
            return false;
        }
    }
    return true;
}

impl_bundle::impl_bundle(const QString& group_name)
    :
      mtx(QMutex::Recursive),
      group_name(group_name),
      saved(group_name),
      transient(saved)
{
}

void impl_bundle::reload()
{
    {
        QMutexLocker l(&mtx);
        saved = group(group_name);
        transient = saved;
    }
    emit reloading();
}

void impl_bundle::store_kv(const QString& name, const QVariant& datum)
{
    QMutexLocker l(&mtx);

    transient.put(name, datum);
}

bool impl_bundle::contains(const QString &name) const
{
    QMutexLocker l(const_cast<QMutex*>(&mtx));
    return transient.contains(name);
}

void impl_bundle::save()
{
    bool modified_ = false;

    {
        QMutexLocker l(&mtx);
        if (saved != transient)
        {
            qDebug() << "bundle" << group_name << "changed, saving";
            modified_ = true;
            saved = transient;
            saved.save();
        }
    }

    if (modified_)
        emit saving();
}

bool impl_bundle::modifiedp() const // XXX unused
{
    QMutexLocker l(const_cast<QMutex*>(&mtx));
    return transient != saved;
}

base_value::base_value(pbundle b, const QString &name) :
    b(b),
    self_name(name)
{
}

opts::~opts()
{
    b->reload();
}

opts::opts(const QString &name) : b(bundle(name))
{
}

custom_type_initializer::custom_type_initializer()
{
    qDebug() << "options: registering stream operators";

    qRegisterMetaTypeStreamOperators<slider_value>("slider_value");
    QMetaType::registerDebugStreamOperator<slider_value>();
}

custom_type_initializer custom_type_initializer::singleton;

namespace detail {

opt_bundle::opt_bundle(const QString& group_name)
    : impl_bundle(group_name)
{
}

opt_bundle::~opt_bundle()
{
    detail::singleton().bundle_decf(group_name);
}

void opt_singleton::bundle_decf(const opt_singleton::k& key)
{
    QMutexLocker l(&implsgl_mtx);

    if (--std::get<0>(implsgl_data[key]) == 0)
    {
        qDebug() << "bundle -" << key;

        implsgl_data.erase(key);
    }
}

opt_singleton::opt_singleton() : implsgl_mtx(QMutex::Recursive)
{
}

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
    implsgl_data[key] = tt(1, shr);
    return shr;
}

OPENTRACK_COMPAT_EXPORT opt_singleton& singleton()
{
    static opt_singleton ret;
    return ret;
}


} // end options::detail

} // end options

