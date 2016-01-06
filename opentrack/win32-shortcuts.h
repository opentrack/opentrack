#pragma once

#ifdef _WIN32

#include <QKeySequence>
#include "opentrack/shortcuts.h"

struct win_key;

extern QList<win_key> windows_key_mods;
extern QList<win_key> windows_key_sequences;

#ifdef BUILD_api
#   include "opentrack-compat/export.hpp"
#else
#   include "opentrack-compat/import.hpp"
#endif

struct OPENTRACK_EXPORT win_key
{
    win_key(int win, Qt::Key qt) : win(win), qt(qt) {}
    int win;
    Qt::Key qt;
    static bool from_qt(QKeySequence qt_, int& dik, Qt::KeyboardModifiers &mods);
    static bool to_qt(const Key& k, QKeySequence& qt_, Qt::KeyboardModifiers &mods);
};

#endif
