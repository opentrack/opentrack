#pragma once

#include <QMetaType>
#include <QList>
#include <QString>
#include <QPointF>
#include <QDataStream>
#include <QDebug>

#include "export.hpp"
#include "slider.hpp"

Q_DECLARE_METATYPE(QList<double>)
Q_DECLARE_METATYPE(QList<float>)
Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<bool>)
Q_DECLARE_METATYPE(QList<QString>)
Q_DECLARE_METATYPE(QList<QPointF>)
Q_DECLARE_METATYPE(::options::slider_value)

namespace options {
namespace detail {

struct custom_type_initializer final
{
    static const custom_type_initializer singleton;

    custom_type_initializer();

    template<typename t> static void declare_for_type(const char* str)
    {
        qRegisterMetaType<t>(str);
        qRegisterMetaTypeStreamOperators<t>();
    }
};

}
}
