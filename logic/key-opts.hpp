#pragma once

#include "options/options.hpp"

struct key_opts
{
    options::value<QString> keycode, guid;
    options::value<int> button;

    inline key_opts(options::bundle b, const QString& name);
    key_opts& operator=(const key_opts& x) = default;
};

key_opts::key_opts(options::bundle b, const QString& name) :
    keycode(b, QString("keycode-%1").arg(name), ""),
    guid(b, QString("guid-%1").arg(name), ""),
    button(b, QString("button-%1").arg(name), -1)
{}
