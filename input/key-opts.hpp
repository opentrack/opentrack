#pragma once

#include "export.hpp"
#include "options/options.hpp"

struct OTR_INPUT_EXPORT key_opts
{
    options::value<QString> keycode, guid;
    options::value<int> button;

    key_opts(options::bundle b, const QString& name);
    key_opts& operator=(const key_opts& x) = default;
};
