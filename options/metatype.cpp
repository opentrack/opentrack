#include "metatype.hpp"

#define OPENTRACK_REGISTER_METATYPE(t) ::options::detail::custom_type_initializer::declare_for_type<t>(#t)

namespace options {
namespace detail {

custom_type_initializer::custom_type_initializer()
{
    OPENTRACK_REGISTER_METATYPE(options::slider_value);
    OPENTRACK_REGISTER_METATYPE(QList<double>);
    OPENTRACK_REGISTER_METATYPE(QList<float>);
    OPENTRACK_REGISTER_METATYPE(QList<int>);
    OPENTRACK_REGISTER_METATYPE(QList<bool>);
    OPENTRACK_REGISTER_METATYPE(QList<QString>);
    OPENTRACK_REGISTER_METATYPE(QList<QPointF>);
}

const custom_type_initializer custom_type_initializer::singleton;

}
}
