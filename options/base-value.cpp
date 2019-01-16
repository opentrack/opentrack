#include "base-value.hpp"

using namespace options;

value_::value_(bundle const& b, const QString& name) noexcept :
    b(b), self_name(name)
{
    b->on_value_created(this);
}

value_::~value_()
{
    b->on_value_destructed(this);
}
