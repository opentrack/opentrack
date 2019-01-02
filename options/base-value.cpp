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

namespace options::detail {

// necessary due to circular dependency
void set_value_to_default(value_* val)
{
    val->set_to_default();
}

} // ns options::detail

