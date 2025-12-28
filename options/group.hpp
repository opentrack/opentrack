#pragma once

#include "options/defs.hpp"

#include "compat/base-path.hpp"
#include "compat/library-path.hpp"
#include "compat/macros.h"
#include "compat/qhash.hpp"
#include "export.hpp"

#include <optional>
#include <unordered_map>

#include <QString>
#include <QVariant>

#include <QDebug>

// XXX TODO remove qsettings usage -sh 20180624

namespace options::detail {
    // snapshot of qsettings group at given time
    class OTR_OPTIONS_EXPORT group final
    {
        QString name;

    public:
        std::unordered_map<QString, QVariant> kvs;
        explicit group(const QString& name);
        void save() const;
        void put(const QString& s, const QVariant& d);
        void put(const QString& s, QVariant&& d);
        bool contains(const QString& s) const;

        tr_never_inline QVariant get_variant(const QString& name) const;
    };
} // ns options::detail


