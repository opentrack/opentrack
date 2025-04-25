#include <QMetaType>

namespace options::detail {

template<typename t>
void declare_metatype_for_type(const char* str)
{
    qRegisterMetaType<t>(str);
}

} // ns options::detail

#define OPENTRACK_DEFINE_METATYPE3(t, ctr)                              \
    static                                                              \
    const char init_##ctr = /* NOLINT(misc-definitions-in-headers) */   \
        (::options::detail::declare_metatype_for_type<t>(#t), 0);       \

#define OPENTRACK_DEFINE_METATYPE2(t, ctr) \
    OPENTRACK_DEFINE_METATYPE3(t, ctr)

#define OPENTRACK_DEFINE_METATYPE(t) \
    OPENTRACK_DEFINE_METATYPE2(t, __COUNTER__)

#define OPENTRACK_METATYPE_(x) OPENTRACK_DEFINE_METATYPE(x)
#include "metatype.hpp"
