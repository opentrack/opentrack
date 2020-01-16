#pragma once

#include <QtGlobal>
#include <QString>
#include <QHashFunctions>

// Qt < 5.14.0
#ifndef QT_SPECIALIZE_STD_HASH_TO_CALL_QHASH_BY_CREF

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
