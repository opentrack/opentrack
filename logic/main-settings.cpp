#include "main-settings.hpp"

namespace main_settings_impl {

using namespace options;

main_settings::main_settings() = default;
module_settings::module_settings() = default;

key_opts::key_opts(bundle b, const QString& name) :
    keycode(b, QString("keycode-%1").arg(name), ""),
    guid(b, QString("guid-%1").arg(name), ""),
    button(b, QString("button-%1").arg(name), -1)
{}

key_opts& key_opts::operator=(const key_opts& x)
{
    if (&x != this)
    {
        keycode = *x.keycode;
        guid = *x.guid;
        button = *x.button;
    }

    return *this;
}

} // ns main_settings_impl
