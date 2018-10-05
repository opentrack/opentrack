#pragma once

#include <QString>
#include <QHashFunctions>

namespace std {
template<> struct hash<QString>
{
    constexpr unsigned operator()(const QString& value) const
    {
        return qHash(value);
    }
};
}
