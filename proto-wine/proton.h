#pragma once

#include <QChar>
#include <QProcess>

std::tuple<QProcessEnvironment, QString, bool> make_steam_environ(const QString& proton_dist_path);
std::tuple<QString, QString, bool> make_wineprefix(long long appid);
