/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "bundle.hpp"
#include "value.hpp"
#include "globals.hpp"

#include <cstdlib>

#include <QThread>
#include <QApplication>

using namespace options;
using namespace options::globals;

namespace options::detail {

bundle::bundle(const QString& group_name) :
      group_name(group_name),
      saved(group_name),
      transient(saved)
{
}

bundle::~bundle() = default;

void bundle::reload()
{
    if (!group_name.isEmpty())
    {
        QMutexLocker l(&mtx);

        // XXX what do we do when values are and aren't equal?
        // see QPointF -sh 20180830

        // XXX we could probably skip assigning to `saved' -sh 20180830
        saved = group(group_name);
        transient = saved;

        connector::notify_all_values();
        emit reloading();
        emit changed();
    }
}

void bundle::set_all_to_default()
{
    QMutexLocker l(&mtx);

    forall([](value_* val) {
        set_value_to_default(val);
    });
}

void bundle::store_kv(const QString& name, const QVariant& new_value)
{
    QMutexLocker l(&mtx);

    if (!group_name.isEmpty())
    {
        transient.put(name, new_value);

        mark_ini_modified();

        connector::notify_values(name);
        emit changed();
    }
}

QVariant bundle::get_variant(const QString& name) const
{
    QMutexLocker l(&mtx);
    return transient.get_variant(name);
}

bool bundle::contains(const QString &name) const
{
    QMutexLocker l(&mtx);
    return transient.contains(name);
}

void bundle::save()
{
    if (QThread::currentThread() != qApp->thread()) // NOLINT
        qDebug() << "group::save - current thread not ui thread";

    if (group_name.isEmpty())
        return;

    {
        QMutexLocker l(&mtx);

        saved = transient;
        saved.save();
    }

    emit saving();
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
    bundler_singleton().after_profile_changed_();
}

bundler::bundler() = default;
bundler::~bundler() = default;

std::shared_ptr<bundler::v> bundler::make_bundle_(const k& key)
{
    QMutexLocker l(&implsgl_mtx);

    using iter = decltype(implsgl_data.cbegin());

    const iter it = implsgl_data.find(key);

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

        const iter it = implsgl_data.find(key);
        if (it != implsgl_data.end())
            (void)implsgl_data.erase(it);
        else
        {
            qCritical() << "ERROR: can't find self-bundle!";
            std::abort();
        }
        delete ptr;
    });
    implsgl_data[key] = weak(shr);
    return shr;
}

bundler& bundler::bundler_singleton()
{
    static bundler ret;
    return ret;
}

} // ns options::detail

namespace options {

std::shared_ptr<bundle_> make_bundle(const QString& name)
{
    if (!name.isEmpty())
        return detail::bundler::bundler_singleton().make_bundle_(name);
    else
        return std::make_shared<bundle_>(QString());
}

} // ns options
