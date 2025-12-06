#pragma once

#include <qchar.h>
#include <qprocess.h>

std::tuple<QProcessEnvironment, QString, bool> make_steam_environ(const QString& proton_dist_path);
std::tuple<QString, QString, bool> make_wineprefix(long long appid);