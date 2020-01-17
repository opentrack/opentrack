#pragma once

#include <QtGlobal>
#include <QString>
#include <QHashFunctions>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)

#include <cstdlib>

namespace std {
template<> struct hash<QString>
{
    std::size_t operator()(const QString& value) const noexcept
    {
        return qHash(value);
    }
};
}

#endif
