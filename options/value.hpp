/* Copyright (c) 2015-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "export.hpp"

#include "bundle.hpp"
#include "slider.hpp"
#include "base-value.hpp"
#include "value-traits.hpp"
#include "compat/macros.hpp"
#include "compat/linkage-macros.hpp"

#include <cstdio>
#include <type_traits>
#include <typeinfo>
#include <typeindex>
#include <utility>

#include <QVariant>
#include <QString>
#include <QPointF>
#include <QList>
#include <QMutex>

namespace options {

template<typename t>
class value final : public value_
{
    using traits = detail::value_traits<t, t, void>;
    using stored_type = typename traits::stored_type;

    static bool is_equal(const QVariant& val1, const QVariant& val2)
    {
        return val1.value<stored_type>() == val2.value<stored_type>();
    }

    cc_noinline
    t get() const
    {
        if (self_name.isEmpty())
            return def;

        QVariant variant = b->get<QVariant>(self_name);

        if (!b->contains(self_name) || variant.type() == QVariant::Invalid)
            return def;

        const stored_type x(variant.value<stored_type>());

        return traits::from_value(traits::from_storage(x), def);
    }

public:
    cc_noinline
    t operator=(const t& datum)
    {
        if (self_name.isEmpty())
            return def;

        if (datum != get())
            store(traits::to_storage(datum));

        return datum;
    }

    static constexpr inline Qt::ConnectionType DIRECT_CONNTYPE = Qt::DirectConnection;
    static constexpr inline Qt::ConnectionType SAFE_CONNTYPE = Qt::QueuedConnection;

    cc_noinline
    value(bundle b, const QString& name, t def) :
        value_(b, name, &is_equal, std::type_index(typeid(stored_type))),
        def(def)
    {
        if (!self_name.isEmpty())
            QObject::connect(b.get(), SIGNAL(reloading()),
                             this, SLOT(reload()),
                             DIRECT_CONNTYPE);
    }

    template<unsigned k>
    inline value(bundle b, const char (&name)[k], t def) : value(b, QLatin1String(name, k-1), def)
    {
        static_assert(k > 0, "");
    }

    cc_noinline
    t default_value() const
    {
        return def;
    }

    cc_noinline
    void set_to_default() override
    {
        *this = def;
    }

    cc_noinline
    operator t() const { return get(); }

    cc_noinline
    t operator->() const
    {
        return get();
    }

    cc_noinline
    void reload() override
    {
        if (!self_name.isEmpty())
            *this = static_cast<t>(*this);
    }

    cc_noinline
    void bundle_value_changed() const override
    {
        if (!self_name.isEmpty())
            emit valueChanged(traits::to_storage(get()));
    }

    cc_noinline
    t operator()() const
    {
        return get();
    }

    template<typename u>
    cc_noinline
    u to() const
    {
        return static_cast<u>(get());
    }

private:
    const t def;
};

#if !defined OTR_INST_VALUE
#   define OTR_INST_VALUE OTR_TEMPLATE_IMPORT
#endif

#if !defined __APPLE__
    OTR_INST_VALUE(value<double>);
    OTR_INST_VALUE(value<float>);
    OTR_INST_VALUE(value<int>);
    OTR_INST_VALUE(value<bool>);
    OTR_INST_VALUE(value<QString>);
    OTR_INST_VALUE(value<slider_value>);
    OTR_INST_VALUE(value<QPointF>);
    OTR_INST_VALUE(value<QVariant>);
    OTR_INST_VALUE(value<QList<double>>);
    OTR_INST_VALUE(value<QList<float>>);
    OTR_INST_VALUE(value<QList<int>>);
    OTR_INST_VALUE(value<QList<bool>>);
    OTR_INST_VALUE(value<QList<QString>>);
    OTR_INST_VALUE(value<QList<slider_value>>);
    OTR_INST_VALUE(value<QList<QPointF>>);
    OTR_INST_VALUE(value<QList<QVariant>>);
#endif

} // ns options
