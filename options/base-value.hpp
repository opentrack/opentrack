#pragma once

#include "bundle.hpp"
#include "slider.hpp"
#include "connector.hpp"

#include "export.hpp"

#include <QObject>
#include <QString>
#include <QList>
#include <QPointF>
#include <QVariant>

#include <typeindex>

#define OPENTRACK_DEFINE_SLOT(t) void setValue(t datum) { store(datum); }
#define OPENTRACK_DEFINE_SIGNAL(t) void valueChanged(t) const

namespace options {

class OTR_OPTIONS_EXPORT base_value : public QObject
{
    Q_OBJECT

    friend class detail::connector;

    using comparator = bool(*)(const QVariant& val1, const QVariant& val2);
    template<typename t>
    using signal_sig = void(base_value::*)(cv_qualified<t>) const;

public:
    bundle get_bundle() { return b; }
    QString name() const { return self_name; }
    base_value(bundle b, const QString& name, comparator cmp, std::type_index type_idx);
    ~base_value() override;

    // MSVC has ODR problems in 15.4
    // no C++17 "constexpr inline" for data declarations in MSVC
    template<typename t>
    constexpr static auto value_changed()
    {
        return static_cast<signal_sig<t>>(&base_value::valueChanged);
    }

    void notify() const;

signals:
    OPENTRACK_DEFINE_SIGNAL(double);
    OPENTRACK_DEFINE_SIGNAL(float);
    OPENTRACK_DEFINE_SIGNAL(int);
    OPENTRACK_DEFINE_SIGNAL(bool);
    OPENTRACK_DEFINE_SIGNAL(const QString&);
    OPENTRACK_DEFINE_SIGNAL(const slider_value&);
    OPENTRACK_DEFINE_SIGNAL(const QPointF&);
    OPENTRACK_DEFINE_SIGNAL(const QVariant&);

    OPENTRACK_DEFINE_SIGNAL(const QList<double>&);
    OPENTRACK_DEFINE_SIGNAL(const QList<float>&);
    OPENTRACK_DEFINE_SIGNAL(const QList<int>&);
    OPENTRACK_DEFINE_SIGNAL(const QList<bool>&);
    OPENTRACK_DEFINE_SIGNAL(const QList<QString>&);
    OPENTRACK_DEFINE_SIGNAL(const QList<slider_value>&);
    OPENTRACK_DEFINE_SIGNAL(const QList<QPointF>&);
protected:
    bundle b;
    QString self_name;
    comparator cmp;
    std::type_index type_index;

    void store(const QVariant& datum);

    template<typename t>
    void store(const t& datum)
    {
        b->store_kv(self_name, QVariant::fromValue(datum));
    }

public slots:
    OPENTRACK_DEFINE_SLOT(double)
    OPENTRACK_DEFINE_SLOT(int)
    OPENTRACK_DEFINE_SLOT(bool)
    OPENTRACK_DEFINE_SLOT(const QString&)
    OPENTRACK_DEFINE_SLOT(const slider_value&)
    OPENTRACK_DEFINE_SLOT(const QPointF&)
    OPENTRACK_DEFINE_SLOT(const QVariant&)

    OPENTRACK_DEFINE_SLOT(const QList<double>&)
    OPENTRACK_DEFINE_SLOT(const QList<float>&)
    OPENTRACK_DEFINE_SLOT(const QList<int>&)
    OPENTRACK_DEFINE_SLOT(const QList<bool>&)
    OPENTRACK_DEFINE_SLOT(const QList<QString>&)
    OPENTRACK_DEFINE_SLOT(const QList<slider_value>&)
    OPENTRACK_DEFINE_SLOT(const QList<QPointF>&)

    virtual void reload() = 0;
    virtual void bundle_value_changed() const = 0;
    virtual void set_to_default() = 0;

    friend void ::options::detail::set_base_value_to_default(base_value* val);
};

} //ns options
