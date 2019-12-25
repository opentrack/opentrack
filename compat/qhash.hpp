#pragma once

#if (QT_VERSION < QT_VERSION_CHECK(5, 14, 0))
#include <QHash>
#include <QString>
#include <functional>

namespace std {
  template<> struct hash<QString> {
    size_t operator()(const QString& s) const noexcept {
      return (size_t) qHash(s);
    }
  };
}
#else
#include <QHashFunctions>
#endif