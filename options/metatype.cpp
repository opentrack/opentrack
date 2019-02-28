#include <QMetaType>
#include "compat/macros.hpp"

namespace options::detail {

template<typename t>
void declare_metatype_for_type(const char* str)
{
    qRegisterMetaType<t>(str);
    qRegisterMetaTypeStreamOperators<t>();
}

} // ns options::detail

#define OPENTRACK_DEFINE_METATYPE2(t, ctr)                                  \
    namespace {                 /* NOLINT(cert-dcl59-cpp) */                \
        static const char ctr = /* NOLINT(misc-definitions-in-headers) */   \
            (::options::detail::declare_metatype_for_type<t>(#t), 0);       \
    } // anon ns

#define OPENTRACK_DEFINE_METATYPE(t) \
    OPENTRACK_DEFINE_METATYPE2(t, PP_CAT(kipple_, __COUNTER__))

#define OPENTRACK_METATYPE_(x) OPENTRACK_DEFINE_METATYPE(x)
#include "metatype.hpp"
