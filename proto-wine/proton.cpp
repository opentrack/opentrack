/* Copyright (c) 2019 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef OTR_WINE_NO_WRAPPER

#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>
#include <errno.h>

#include <memory>

#include <QtGlobal>
#include <QString>
#include <QProcessEnvironment>
#include <QDebug>

static QString home_directory()
{
    struct passwd *pwptr = nullptr, pw{};
    uid_t uid = getuid();
    int shift = 16;
    do {
        auto buf = std::make_unique<char[]>(1 << shift);
        int error = getpwuid_r(uid, &pw, buf.get(), 1 << shift, &pwptr);
        if (error)
        {
            if (errno != EDOM)
                return {};
            else
                continue;
        }
        else
            break;
    } while (++shift < 28);

    if (!pwptr)
        return {};

    return pwptr->pw_dir;
}

QProcessEnvironment make_steam_environ(const QString& proton_version)
{
    auto ret = QProcessEnvironment::systemEnvironment();
    QString home = home_directory();

    auto expand = [&](QString x) {
        x.replace("HOME", home);
        x.replace("PROTON", proton_version);
        return x;
    };

    QString path = expand(
        "HOME/.local/share/Steam/steamapps/common/Proton PROTON/dist/bin:"
        "HOME/.local/share/Steam/ubuntu12_32/steam-runtime/amd64/bin:"
        "HOME/.local/share/Steam/ubuntu12_32/steam-runtime/amd64/usr/bin"
    );
    path += ':'; path += qgetenv("PATH");
    ret.insert("PATH", path);

    QString library_path = expand(
        ":HOME/.local/share/Steam/steamapps/common/Proton PROTON/dist/lib"
        ":HOME/.local/share/Steam/steamapps/common/Proton PROTON/dist/lib64"
        ":HOME/.local/share/Steam/ubuntu12_32/steam-runtime/pinned_libs_32"
        ":HOME/.local/share/Steam/ubuntu12_32/steam-runtime/pinned_libs_64"
        ":HOME/.local/share/Steam/ubuntu12_32/steam-runtime/i386/lib/i386-linux-gnu"
        ":HOME/.local/share/Steam/ubuntu12_32/steam-runtime/i386/lib"
        ":HOME/.local/share/Steam/ubuntu12_32/steam-runtime/i386/usr/lib/i386-linux-gnu"
        ":HOME/.local/share/Steam/ubuntu12_32/steam-runtime/i386/usr/lib"
        ":HOME/.local/share/Steam/ubuntu12_32/steam-runtime/amd64/lib/x86_64-linux-gnu"
        ":HOME/.local/share/Steam/ubuntu12_32/steam-runtime/amd64/lib"
        ":HOME/.local/share/Steam/ubuntu12_32/steam-runtime/amd64/usr/lib/x86_64-linux-gnu"
        ":HOME/.local/share/Steam/ubuntu12_32/steam-runtime/amd64/usr/lib"
    );
    library_path += ':'; library_path += qgetenv("LD_LIBRARY_PATH");
    ret.insert("LD_LIBRARY_PATH", library_path);

    ret.insert("WINEPREFIX", expand("HOME/.local/share/Steam/steamapps/compatdata/805550/pfx"));
    ret.insert("WINEESYNC", "1");

    return ret;
}

QString proton_path(const QString& proton_version)
{
    QString wine_path = "HOME/.local/share/Steam/steamapps/common/Proton PROTON/dist/bin/wine";
    wine_path.replace("HOME", home_directory()); wine_path.replace("PROTON", proton_version);
    return wine_path;
}

#endif
