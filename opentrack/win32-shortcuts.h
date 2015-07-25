#pragma once

#ifdef _WIN32

struct win_key;

extern QList<win_key> windows_key_mods;
extern QList<win_key> windows_key_sequences;

struct win_key
{
    win_key(int win, Qt::Key qt) : win(win), qt(qt) {}
    int win;
    Qt::Key qt;
    static bool from_qt(QKeySequence qt_, int& dik, Qt::KeyboardModifiers &mods);
};

#endif
