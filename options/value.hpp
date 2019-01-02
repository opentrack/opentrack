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
#include <utility>

#include <QMetaType>

namespace options::detail {
    template<typename t>
    class dereference_wrapper final
    {
        t x;
    public:
        constexpr t const* operator->() const { return &x; }
        constexpr t* operator->() { return &x; }
        constexpr explicit dereference_wrapper(t&& x) : x(x) {}
    };
} // ns options::detail

namespace options {

template<typename u>
class value final : public value_
{
    using t = remove_cvref_t<u>;
    const t def;

    using traits = detail::value_traits<t>;

    cc_noinline
    t get() const noexcept
    {
        if (self_name.isEmpty() || !b->contains(self_name))
            return traits::pass_value(def);

        QVariant variant = b->get_variant(self_name);

        if (variant.isNull() || !variant.isValid())
            return traits::pass_value(def);

        return traits::pass_value(traits::value_with_default(traits::value_from_qvariant(variant), def));
    }

    cc_noinline
    void store_variant(const QVariant& value) noexcept override
    {
        if (self_name.isEmpty())
            return;

        if (traits::is_equal(get(), traits::value_from_qvariant(value)))
            return;

        if (value.isValid() && !value.isNull())
            b->store_kv(self_name, value);
        else
            b->store_kv(self_name, traits::qvariant_from_value(def));
    }

public:
    void notify() const override
    {
        if (!self_name.isEmpty())
            emit valueChanged(traits::storage_from_value(get()));
    }

    value<u>& operator=(const t& datum) noexcept
    {
        store_variant(traits::qvariant_from_value(traits::pass_value(datum)));

        return *this;
    }

    static constexpr inline Qt::ConnectionType DIRECT_CONNTYPE = Qt::DirectConnection;
    static constexpr inline Qt::ConnectionType SAFE_CONNTYPE = Qt::QueuedConnection;

    value(bundle b, const QString& name, t def) noexcept : value_(b, name), def(std::move(def))
    {
    }

    value(const value<u>& other) noexcept : value{other.b, other.self_name, other.def} {}

    t default_value() const
    {
        return def;
    }

    void set_to_default() override
    {
        *this = def;
    }

    operator t() const { return get(); } // NOLINT

    template<typename w>
    explicit cc_forceinline operator w() const { return to<w>(); }

    auto operator->() const noexcept
    {
        return detail::dereference_wrapper<t>{get()};
    }

    cc_forceinline t operator()() const noexcept { return get(); }
    cc_forceinline t operator*() const noexcept { return get(); }

    template<typename w>
    w to() const noexcept
    {
        return static_cast<w>(get());
    }
};

// some linker problems
#if !defined OTR_INST_VALUE
#   define OTR_INST_VALUE OTR_TEMPLATE_IMPORT
#endif

OTR_INST_VALUE(value<double>)
OTR_INST_VALUE(value<float>)
OTR_INST_VALUE(value<int>)
OTR_INST_VALUE(value<bool>)
OTR_INST_VALUE(value<QString>)
OTR_INST_VALUE(value<slider_value>)
OTR_INST_VALUE(value<QPointF>)
OTR_INST_VALUE(value<QVariant>)
OTR_INST_VALUE(value<QList<double>>)
OTR_INST_VALUE(value<QList<float>>)
OTR_INST_VALUE(value<QList<int>>)
OTR_INST_VALUE(value<QList<bool>>)
OTR_INST_VALUE(value<QList<QString>>)
OTR_INST_VALUE(value<QList<slider_value>>)
OTR_INST_VALUE(value<QList<QPointF>>)
OTR_INST_VALUE(value<QList<QVariant>>)

} // ns options
