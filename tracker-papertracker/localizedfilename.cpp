#include "options/options.hpp"
#include "localizedfilename.h"
#include <QFile>
#include <QLocale>
#include <QSettings>
#include <QStringList>

namespace papertracker {
    QString localized_filename(const QString &location, const QString& filename) {
        const bool no_i18n = options::globals::with_global_settings_object([](QSettings& s) {
            return s.value("disable-translation", false).toBool();
        });

        QStringList candidates;

        if (!no_i18n) {
            for (const QString& ui : QLocale().uiLanguages()) {
                const QLocale locale(ui);
                const QString lang = QLocale::languageToCode(locale.language());
                const QString country = QLocale::countryToCode(locale.country());

                if (lang.isEmpty())
                    continue;

                if (!country.isEmpty())
                    candidates << lang + QLatin1Char('_') + country;

                candidates << lang;
            }
        }

        candidates << QStringLiteral("en");
        candidates.removeDuplicates();

        for (const QString& language : std::as_const(candidates)) {
            const QString path = QStringLiteral("%1/%2/%3").arg(location, language, filename);
            if (QFile::exists(path))
                return path;
        }

        return {};
    }
}