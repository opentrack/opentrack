/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "bundle.hpp"
#include "value.hpp"

#include <QThread>
#include <QApplication>

using options::base_value;

namespace options
{

namespace detail {

bundle::bundle(const QString& group_name)
    : mtx(QMutex::Recursive),
      group_name(group_name),
      saved(group_name),
      transient(saved)
{
}

bundle::~bundle()
{
}

void bundle::reload()
{
    if (group_name.size())
    {
        QMutexLocker l(&mtx);
        saved = group(group_name);
        const bool has_changes = is_modified();
        transient = saved;

        if (has_changes)
        {
            connector::notify_all_values();
            emit reloading();
            emit changed();
        }
    }
}

void bundle::set_all_to_default()
{
    QMutexLocker l(&mtx);

    forall([](const QString&, base_value* val) { set_base_value_to_default(val); });

    if (is_modified())
        group::mark_ini_modified();
}

void bundle::store_kv(const QString& name, const QVariant& datum)
{
    QMutexLocker l(&mtx);

    transient.put(name, datum);

    if (group_name.size())
        connector::notify_values(name);

    emit changed();
}

bool bundle::contains(const QString &name) const
{
    QMutexLocker l(mtx);
    return transient.contains(name);
}

void bundle::save()
{
    if (QThread::currentThread() != qApp->thread())
        qDebug() << "group::save - current thread not ui thread";

    if (group_name.size() == 0)
        return;

    bool modified_ = false;

    {
        QMutexLocker l(&mtx);

        if (is_modified())
        {
            //qDebug() << "bundle" << group_name << "changed, saving";
            modified_ = true;
            saved = transient;
            saved.save();
        }
    }

    if (modified_)
        emit saving();
}

bool bundle::is_modified() const
{
    QMutexLocker l(mtx);

    for (const auto& kv : transient.kvs)
    {
        const QVariant other = saved.get<QVariant>(kv.first);
        if (!saved.contains(kv.first) || !is_equal(kv.first, kv.second, other))
        {
            //if (logspam)
            //    qDebug() << "bundle" << group_name << "modified" << "key" << kv.first << "-" << other << "<>" << kv.second;
            return true;
        }
    }

    for (const auto& kv : saved.kvs)
    {
        if (!transient.contains(kv.first))
        {
            //if (logspam)
            //    qDebug() << "bundle" << group_name << "modified" << "key" << kv.first << "-" << other << "<>" << kv.second;
            return true;
        }
    }

    return false;
}

void bundler::after_profile_changed_()
{
    QMutexLocker l(&implsgl_mtx);

    for (auto& kv : implsgl_data)
    {
        weak bundle = kv.second;
        shared bundle_ = bundle.lock();
        if (bundle_)
        {
            //qDebug() << "bundle: reverting" << kv.first << "due to profile change";
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
    //qDebug() << "exit: bundle singleton";
}

std::shared_ptr<bundler::v> bundler::make_bundle(const bundler::k& key)
{
    QMutexLocker l(&implsgl_mtx);

    auto it = implsgl_data.find(key);

    if (it != implsgl_data.end())
    {
        std::shared_ptr<v> ptr = it->second.lock();
        if (ptr != nullptr)
            return ptr;
        else
            qDebug() << "ERROR: nonexistent bundle" << key;
    }

    auto shr = shared(new v(key), [this, key](v* ptr) {
        QMutexLocker l(&implsgl_mtx);

        auto it = implsgl_data.find(key);
        if (it != implsgl_data.end())
            implsgl_data.erase(it);
        else
            qDebug() << "ERROR: can't find self-bundle!";
        delete ptr;
    });
    implsgl_data[key] = weak(shr);
    return shr;
}

OTR_OPTIONS_EXPORT bundler& singleton()
{
    static bundler ret;
    return ret;
}

QMutex* bundle::get_mtx() const { return mtx; }

} // end options::detail

OTR_OPTIONS_EXPORT std::shared_ptr<bundle_> make_bundle(const QString& name)
{
    if (name.size())
        return detail::singleton().make_bundle(name);
    else
        return std::make_shared<bundle_>(QString());
}

} // end options
