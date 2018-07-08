#pragma once

#include "options/defs.hpp"

#include "compat/base-path.hpp"
#include "compat/library-path.hpp"
#include "compat/qhash.hpp"
#include "compat/macros.hpp"
#include "export.hpp"

#include <optional>
#include <unordered_map>

#include <QHash>
#include <QString>
#include <QMutex>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QVariant>
#include <QSettings>
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
        bool contains(const QString& s) const;

        template<typename t>
        cc_noinline t get(const QString& k) const
        {
            return get_variant(k).value<t>();
        }

        cc_noinline QVariant get_variant(const QString& name) const;
    };
} // ns options::detail


