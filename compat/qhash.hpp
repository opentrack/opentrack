#pragma once

#include <QtGlobal>
#include <QString>
#include <QHashFunctions>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)

namespace std {
template<> struct hash<QString>
{
    unsigned operator()(const QString& value) const
    {
        return qHash(value);
    }
};
}

#endif
