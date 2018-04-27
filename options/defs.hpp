#pragma once

#include <QString>
#include <QMetaType>

#define OPENTRACK_CONFIG_FILENAME_KEY "settings-filename"
#define OPENTRACK_DEFAULT_CONFIG "default.ini"
#define OPENTRACK_DEFAULT_CONFIG_Q QStringLiteral("default.ini")
#define OTR_OPTIONS_EXPAND2(x) x
#define OTR_OPTIONS_EXPAND1(x) OTR_OPTIONS_EXPAND2(x)

#define OPENTRACK_REGISTER_METATYPE2(t, ctr)                                                        \
    OPENTRACK_REGISTER_METATYPE3(t, ctr)

#define OPENTRACK_REGISTER_METATYPE3(t, sym)                                                        \
    OPENTRACK_REGISTER_METATYPE4(t, init_metatype_ ## sym)

#define OPENTRACK_REGISTER_METATYPE4(t, sym)                                                        \
    class sym {                                                                                     \
        static const int dribble;                                                                   \
    } sym ## _singleton;                                                                            \
    const int sym :: dribble = ::options::detail::custom_type_initializer::declare_for_type<t>(#t)

#if defined Q_CREATOR_RUN
#   define OPENTRACK_DEFINE_METATYPE(t)
#else
#   define OPENTRACK_DEFINE_METATYPE(t) OPENTRACK_REGISTER_METATYPE2(t, OTR_OPTIONS_EXPAND1(__COUNTER__))
#endif

namespace options {
namespace detail {

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
} // ns options
