#include "bundle.hpp"

namespace options
{

namespace detail {

bundle::bundle(const QString& group_name)
    :
      mtx(QMutex::Recursive),
      group_name(group_name),
      saved(group_name),
      transient(saved)
{
}

void bundle::reload()
{
    {
        QMutexLocker l(&mtx);
        saved = group(group_name);
        transient = saved;
    }
    emit reloading();
}

void bundle::store_kv(const QString& name, const QVariant& datum)
{
    QMutexLocker l(&mtx);

    transient.put(name, datum);
}

bool bundle::contains(const QString &name) const
{
    QMutexLocker l(const_cast<QMutex*>(&mtx));
    return transient.contains(name);
}

void bundle::save_deferred(QSettings& s)
{
    if (group_name == "")
        return;

    bool modified_ = false;

    {
        QMutexLocker l(&mtx);
        if (saved != transient)
        {
            qDebug() << "bundle" << group_name << "changed, saving";
            modified_ = true;
            saved = transient;
            saved.save_deferred(s);
        }
    }

    if (modified_)
        emit saving();
}

void bundle::save()
{
    save_deferred(*group::ini_file());
}

bool bundle::modifiedp() const // XXX unused
{
    QMutexLocker l(const_cast<QMutex*>(&mtx));
    return transient != saved;
}

void bundler::bundle_decf(const bundler::k& key)
{
    QMutexLocker l(&implsgl_mtx);

    if (--std::get<0>(implsgl_data[key]) == 0)
    {
        qDebug() << "bundle -" << key;

        implsgl_data.erase(key);
    }
}

void bundler::after_profile_changed_()
{
    QMutexLocker l(&implsgl_mtx);

    for (auto& kv : implsgl_data)
    {
        tt& tuple = kv.second;
        std::weak_ptr<v>& bundle = std::get<1>(tuple);

        mem<v> bundle_ = bundle.lock();
        if (bundle_)
        {
            qDebug() << "bundle: reverting" << kv.first << "due to profile change";
            bundle_->reload();
        }
    }
}

void bundler::refresh_all_bundles()
{
    singleton().after_profile_changed_();
}

bundler::bundler() : implsgl_mtx(QMutex::Recursive)
{
}

bundler::~bundler()
{
    qDebug() << "exit: bundle singleton";
}

std::shared_ptr<bundler::v> bundler::make_bundle(const bundler::k &key)
{
    QMutexLocker l(&implsgl_mtx);

    if (implsgl_data.count(key) != 0)
    {
        auto shared = std::get<1>(implsgl_data[key]).lock();
        if (shared != nullptr)
            return shared;
    }

    qDebug() << "bundle +" << key;

    std::shared_ptr<v> shr(new v(key), [this](v* val) { bundle_decf(val->name()); });

    implsgl_data[key] = tt(1, shr);
    return shr;
}

OPENTRACK_OPTIONS_EXPORT bundler& singleton()
{
    static bundler ret;
    return ret;
}

} // end options::detail

} // end options

