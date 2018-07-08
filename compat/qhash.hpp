#pragma once

#include <functional>
#include <QString>
#include <QHashFunctions>

namespace std {
template<> struct hash<QString>
{
    inline unsigned operator()(const QString& value) const
    {
        return qHash(value);
    }
};
}
