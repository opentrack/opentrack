#include <QMetaType>
#include "compat/macros.hpp"

namespace options::detail {

template<typename t>
int declare_metatype_for_type(const char* str)
{
    qRegisterMetaType<t>(str);
    qRegisterMetaTypeStreamOperators<t>();

    return -1;
}

} // ns options::detail

#define OPENTRACK_DEFINE_METATYPE2(t, sym)                                  \
    class sym { /* NOLINT */                                                \
        static const int dribble;                                           \
    }; /* NOLINT */                                                         \
    static sym sym;                                                         \
    const int sym :: dribble = ::options::detail::declare_metatype_for_type<t>(#t);

#define OPENTRACK_DEFINE_METATYPE(t) \
    OPENTRACK_DEFINE_METATYPE2(t, PP_CAT(init, __COUNTER__))

#define OPENTRACK_METATYPE_(x) OPENTRACK_DEFINE_METATYPE(x)
#include "metatype.hpp"
