/* Copyright (c) 2019 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef OTR_WINE_NO_WRAPPER

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QtGlobal>

#include "proton.h"

static const char* steam_paths[] = {
    "/.steam/steam/steamapps/compatdata",
    "/.local/share/Steam/steamapps/compatdata",
    "/.steam/debian-installation/steamapps/compatdata",
};

static const char* runtime_paths[] = {
    "/.local/share/Steam/ubuntu12_32/steam-runtime",
    "/.steam/ubuntu12_32/steam-runtime",
    "/.steam/debian-installation/ubuntu12_32/steam-runtime",
};


std::tuple<QProcessEnvironment, QString, bool> make_steam_environ(const QString& proton_dist_path)
{
    using ret = std::tuple<QProcessEnvironment, QString, bool>;
    auto env = QProcessEnvironment::systemEnvironment();
    QString error = "";
    QString home = qgetenv("HOME");
    QString runtime_path;

    auto expand = [&](QString x) {
                      x.replace("HOME", home);
                      x.replace("PROTON_DIST_PATH", proton_dist_path);
                      x.replace("RUNTIME_PATH", runtime_path);
                      return x;
                  };

    for (const char* path : runtime_paths) {
        QDir dir(QDir::homePath() + path);
        if (dir.exists())
            runtime_path = dir.absolutePath();
    }

    if (runtime_path.isEmpty())
        error = QString("Couldn't find a Steam runtime.");

    QString path = expand(
        ":PROTON_DIST_PATH/bin"
    );
    path += ':'; path += qgetenv("PATH");
    env.insert("PATH", path);

    QString library_path = expand(
        ":PROTON_DIST_PATH/lib"
        ":PROTON_DIST_PATH/lib64"
        ":RUNTIME_PATH/pinned_libs_32"
        ":RUNTIME_PATH/pinned_libs_64"
        ":RUNTIME_PATH/i386/lib/i386-linux-gnu"
        ":RUNTIME_PATH/i386/lib"
        ":RUNTIME_PATH/i386/usr/lib/i386-linux-gnu"
        ":RUNTIME_PATH/i386/usr/lib"
        ":RUNTIME_PATH/amd64/lib/x86_64-linux-gnu"
        ":RUNTIME_PATH/amd64/lib"
        ":RUNTIME_PATH/amd64/usr/lib/x86_64-linux-gnu"
        ":RUNTIME_PATH/amd64/usr/lib"
    );
    library_path += ':'; library_path += qgetenv("LD_LIBRARY_PATH");
    env.insert("LD_LIBRARY_PATH", library_path);

    return ret(env, error, error.isEmpty());
}


std::tuple<QString, QString, bool> make_wineprefix(long appid)
{
    using ret = std::tuple<QString, QString, bool>;
    QString error = "";
    QString app_wineprefix;
    for (const char* path : steam_paths) {
        QDir dir(QDir::homePath() + path + QString("/%1/pfx").arg(appid));
        if (dir.exists())
            app_wineprefix = dir.absolutePath();
    }
    if (app_wineprefix.isEmpty())
        error = QString("Couldn't find a Wineprefix for AppId %1").arg(appid);

    return ret(app_wineprefix, error, error.isEmpty());
}

#endif
