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

template<typename t>
class value final : public value_
{
    static_assert(std::is_same_v<t, std::remove_cvref_t<t>>);
    mutable QMutex mtx;
    const t def;
    mutable t cached_value;
    using traits = detail::value_traits<t>;

    tr_never_inline
    auto get() const noexcept
    {
        if (!b->contains(self_name))
            return traits::pass_value(def);

        QVariant variant = b->get_variant(self_name);

        if (variant.isNull() || !variant.isValid())
            return traits::pass_value(def);

        return traits::pass_value(traits::value_with_default(traits::value_from_qvariant(variant), def));
    }

    tr_never_inline
    void store_variant(QVariant&& value) noexcept override
    {
        if (traits::is_equal(get(), traits::value_from_qvariant(value)))
            return;

        if (is_null())
            return;

        if (value.isValid() && !value.isNull())
            b->store_kv(self_name, value);
        else
            b->store_kv(self_name, traits::qvariant_from_value(def));
    }

    tr_never_inline
    void store_variant(const QVariant& value) noexcept override
    {
        QVariant copy{value};
        store_variant(std::move(copy));
    }

    bool is_null() const
    {
        return self_name.isEmpty() || b->name().isEmpty();
    }

public:
    QVariant get_variant() const noexcept override
    {
        if (QVariant ret{b->get_variant(self_name)}; ret.isValid() && !ret.isNull())
            return ret;

        return traits::qvariant_from_value(def);
    }

    tr_never_inline
    void notify_() const override
    {
        auto x = get();
        {
            QMutexLocker l(&mtx);
            cached_value = x;
        }
        maybe_trace("notify +");
        emit valueChanged(traits::storage_from_value(x));
        maybe_trace("notify -");
    }

    tr_never_inline
    void notify() const override
    {
        if (is_null())
            return;

        auto x = get();
        {
            QMutexLocker l(&mtx);
            if (traits::is_equal(x, cached_value))
            {
                //maybe_trace("notify ~");
                return;
            }
            else
            {
                cached_value = x;
                l.unlock();
                maybe_trace("notify +");
                emit valueChanged(traits::storage_from_value(x));
                maybe_trace("notify -");
            }
        }
    }

    auto& operator=(t&& datum) noexcept
    {
        if (is_null())
            return *this;

        store_variant(traits::qvariant_from_value(traits::pass_value(datum)));
        maybe_trace("set-value");
        return *this;
    }

    auto& operator=(const t& datum) noexcept
    {
        if (is_null())
            return *this;

        t copy{datum};
        *this = std::move(copy);
        return *this;
    }

    auto& operator=(const value<t>& datum) noexcept
    {
        *this = *datum;
        return *this;
    }

    static constexpr Qt::ConnectionType DIRECT_CONNTYPE = Qt::DirectConnection;
    static constexpr Qt::ConnectionType SAFE_CONNTYPE = Qt::QueuedConnection;

    value(bundle b, const QString& name, t def) noexcept : value_(b, name), def(std::move(def)), cached_value{get()}
    {
    }

    //value(const value<t>& other) noexcept : value{other.b, other.self_name, other.def} {}
    value(const value<t>&) = delete;

    t default_value() const
    {
        return def;
    }

    void set_to_default() noexcept override
    {
        *this = def;
    }

    operator t() const { return get(); }

    template<typename u>
    explicit tr_force_inline operator u() const { return to<u>(); }

    auto operator->() const noexcept
    {
        return detail::dereference_wrapper<t>{get()};
    }

    tr_force_inline auto operator()() const noexcept { return get(); }
    tr_force_inline auto operator*() const noexcept { return get(); }

    template<typename u>
    u to() const noexcept
    {
        return static_cast<u>(get());
    }
};

#if !defined OTR_OPTIONS_INST_VALUE
#   define OTR_OPTIONS_INST_VALUE OTR_TEMPLATE_IMPORT
#endif

OTR_OPTIONS_INST_VALUE(value<double>)
OTR_OPTIONS_INST_VALUE(value<float>)
OTR_OPTIONS_INST_VALUE(value<int>)
OTR_OPTIONS_INST_VALUE(value<bool>)
OTR_OPTIONS_INST_VALUE(value<QString>)
OTR_OPTIONS_INST_VALUE(value<slider_value>)
OTR_OPTIONS_INST_VALUE(value<QPointF>)
OTR_OPTIONS_INST_VALUE(value<QVariant>)
OTR_OPTIONS_INST_VALUE(value<QList<double>>)
OTR_OPTIONS_INST_VALUE(value<QList<float>>)
OTR_OPTIONS_INST_VALUE(value<QList<int>>)
OTR_OPTIONS_INST_VALUE(value<QList<bool>>)
OTR_OPTIONS_INST_VALUE(value<QList<QString>>)
OTR_OPTIONS_INST_VALUE(value<QList<slider_value>>)
OTR_OPTIONS_INST_VALUE(value<QList<QPointF>>)
OTR_OPTIONS_INST_VALUE(value<QList<QVariant>>)

} // ns options
