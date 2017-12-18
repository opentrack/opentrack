/* Copyright (c) 2015-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "export.hpp"

#include "compat/util.hpp"

#include "bundle.hpp"
#include "slider.hpp"
#include "base-value.hpp"
#include "value-traits.hpp"

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
class value final : public base_value
{
    using traits = detail::value_traits<t, t, void>;
    using element_type = typename traits::element_type;

    static bool is_equal(const QVariant& val1, const QVariant& val2)
    {
        return val1.value<element_type>() == val2.value<element_type>();
    }

    never_inline
    t get() const
    {
        if (self_name.isEmpty())
            return def;

        if (!b->contains(self_name) || b->get<QVariant>(self_name).type() == QVariant::Invalid)
            return def;

        const element_type x(b->get<element_type>(self_name));

        return traits::from_value(traits::from_storage(x), def);
    }

public:
    never_inline
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

    never_inline
    value(bundle b, const QString& name, t def) :
        base_value(b, name, &is_equal, std::type_index(typeid(element_type))),
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

    never_inline
    t default_value() const
    {
        return def;
    }

    never_inline
    void set_to_default() override
    {
        *this = def;
    }

    never_inline
    operator t() const { return std::forward<t>(get()); }

    never_inline
    t operator->() const
    {
        return get();
    }

    never_inline
    void reload() override
    {
        if (!self_name.isEmpty())
            *this = static_cast<t>(*this);
    }

    never_inline
    void bundle_value_changed() const override
    {
        if (!self_name.isEmpty())
            emit valueChanged(traits::to_storage(get()));
    }

    never_inline
    t operator()() const
    {
        return get();
    }

    template<typename u>
    never_inline
    u to() const
    {
        return static_cast<u>(std::forward<t>(get()));
    }

private:
    const t def;
};

#if defined _MSC_VER

#   if !defined OTR_OPT_VALUE
#      define OTR_OPT_VALUE OTR_TEMPLATE_IMPORT
#   endif

    OTR_OPT_VALUE value<double>;
    OTR_OPT_VALUE value<float>;
    OTR_OPT_VALUE value<int>;
    OTR_OPT_VALUE value<bool>;
    OTR_OPT_VALUE value<QString>;
    OTR_OPT_VALUE value<slider_value>;
    OTR_OPT_VALUE value<QPointF>;
    OTR_OPT_VALUE value<QVariant>;

    OTR_OPT_VALUE value<QList<double>>;
    OTR_OPT_VALUE value<QList<float>>;
    OTR_OPT_VALUE value<QList<int>>;
    OTR_OPT_VALUE value<QList<bool>>;
    OTR_OPT_VALUE value<QList<QString>>;
    OTR_OPT_VALUE value<QList<slider_value>>;
    OTR_OPT_VALUE value<QList<QPointF>>;

#endif

} // ns options
