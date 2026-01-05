#pragma once

#include "export.hpp"
#include <QKeySequence>
#include "input/key.hpp"

struct OTR_INPUT_EXPORT win_key
{
    Qt::Key qt;
    unsigned char win;
    unsigned char vk;

    [[nodiscard]] static bool qt_to_dik(const QKeySequence& qt_, int& dik, Qt::KeyboardModifiers &mods);
    [[nodiscard]] static bool dik_to_qt(const Key& k, QKeySequence& qt, Qt::KeyboardModifiers &mods);

    [[nodiscard]] static bool qt_to_vk(const QKeySequence& qt, int& vk, Qt::KeyboardModifiers& mods);
    [[nodiscard]] static bool vk_to_qt(const Key& k, QKeySequence& vk, Qt::KeyboardModifiers& mods);
};
