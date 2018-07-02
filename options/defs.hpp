#pragma once

#include "compat/macros.hpp"

#include <QString>
#include <QMetaType>

#define OPENTRACK_CONFIG_FILENAME_KEY "settings-filename"
#define OPENTRACK_DEFAULT_CONFIG "default.ini"
#define OTR_OPTIONS_EXPAND2(x) x
#define OTR_OPTIONS_EXPAND1(x) OTR_OPTIONS_EXPAND2(x)

#define OPENTRACK_DEFINE_METATYPE2(t, ctr)                                                          \
    OPENTRACK_DEFINE_METATYPE3(t, ctr)

#define OPENTRACK_DEFINE_METATYPE3(t, ctr)                                                          \
    OPENTRACK_DEFINE_METATYPE4(t, init_metatype_ ## ctr)

#define OPENTRACK_DEFINE_METATYPE4(t, sym)                                                          \
    static class sym {                                                                              \
        static const int dribble;                                                                   \
    } sym ## _singleton;                                                                            \
    const int sym :: dribble = ::options::detail::custom_type_initializer::declare_for_type<t>(#t);

#define OPENTRACK_DEFINE_METATYPE(t) OPENTRACK_DEFINE_METATYPE2(t, __COUNTER__)

namespace options::detail {

struct custom_type_initializer final
{
    template<typename t> static int declare_for_type(const char* str)
    {
        qRegisterMetaType<t>(str);
        qRegisterMetaTypeStreamOperators<t>();

        return -1;
    }
};

} // ns options::detail

