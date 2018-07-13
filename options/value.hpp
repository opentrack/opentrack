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

#include <type_traits>
#include <typeinfo>

#include <QMetaType>

namespace options::detail {
    template<typename t>
    struct dereference_wrapper final
    {
        cc_forceinline constexpr t const* operator->() const { return &x; }
        cc_forceinline constexpr t* operator->() { return &x; }
        t x;
        constexpr explicit cc_forceinline dereference_wrapper(t&& x) : x(x) {}
    };
} // ns options::detail

namespace options {

template<typename t>
class value final : public value_
{
    const t def;

    using traits = detail::value_traits<t>;
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

        const stored_type x { variant.value<stored_type>() };

        return traits::from_value(traits::from_storage(x), def);
    }

    friend class detail::connector;
    void bundle_value_changed() const override
    {
        if (!self_name.isEmpty())
            emit valueChanged(traits::to_storage(get()));
    }

    void store_variant(const QVariant& value) override
    {
        if (self_name.isEmpty())
            return;

        if (value.type() == qMetaTypeId<stored_type>())
            b->store_kv(self_name, value);
        else
            operator=(traits::value_from_variant(value));
    }

public:
    cc_noinline
    value<t>& operator=(const t& datum)
    {
        if (self_name.isEmpty())
            return *this;

        if (datum != get())
            b->store_kv(self_name, QVariant::fromValue<stored_type>(traits::to_storage(datum)));

        return *this;
    }

    static constexpr inline Qt::ConnectionType DIRECT_CONNTYPE = Qt::DirectConnection;
    static constexpr inline Qt::ConnectionType SAFE_CONNTYPE = Qt::QueuedConnection;

    cc_noinline
    value(bundle b, const QString& name, t def) :
        value_(b, name, &is_equal, std::type_index(typeid(stored_type))),
        def(def)
    {
        if (!self_name.isEmpty())
            QObject::connect(b.get(), &detail::bundle::reloading,
                             this, &value_::reload,
                             DIRECT_CONNTYPE);
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

    operator t() const { return get(); } // NOLINT

    template<typename u, typename = decltype(static_cast<u>(std::declval<t>()))>
    explicit cc_forceinline operator u() const { return to<u>(); }

    auto operator->() const
    {
        return detail::dereference_wrapper<t>{get()};
    }

    cc_noinline
    void reload() override
    {
#if 0
        if (!self_name.isEmpty())
            store(traits::to_storage(get()));
#endif
    }

    cc_forceinline t operator()() const { return get(); }
    cc_forceinline t operator*() const { return get(); }

    template<typename u>
    u to() const
    {
        return static_cast<u>(get());
    }
};

// some linker problems
#if !defined __APPLE__
#   if !defined OTR_INST_VALUE
#       define OTR_INST_VALUE OTR_TEMPLATE_IMPORT
#   endif

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
