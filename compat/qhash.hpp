#pragma once

#include <QString>
#include <QHashFunctions>

namespace std {
template<> struct hash<QString>
{
    unsigned operator()(const QString& value) const
    {
        return qHash(value);
    }
};
}
