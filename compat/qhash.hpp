#pragma once

#include <functional>

#include <QtGlobal>
#include <QString>
#include <QHashFunctions>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)

#include <cstdlib>

namespace std {
template<> struct hash<QString>
{
    using argument_type = QString;
    using result_type = std::size_t;

    std::size_t operator()(const QString& value) const noexcept
    {
        return (std::size_t) qHash(value);
    }
};
}

#endif
