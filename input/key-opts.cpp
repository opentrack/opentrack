#include "key-opts.hpp"

key_opts::key_opts(options::bundle b, const QString& name) :
    keycode(b, QString("keycode-%1").arg(name), ""),
    guid(b, QString("guid-%1").arg(name), ""),
    button(b, QString("button-%1").arg(name), -1)
{}
