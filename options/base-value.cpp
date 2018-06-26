#include "base-value.hpp"

using namespace options;

value_::value_(bundle b, const QString& name, value_::comparator cmp, std::type_index type_idx) :
    b(b),
    self_name(name),
    cmp(cmp),
    type_index(type_idx)
{
    b->on_value_created(name, this);
}

value_::~value_()
{
    b->on_value_destructed(self_name, this);
}

void value_::notify() const
{
    bundle_value_changed();
}

void value_::store(const QVariant& datum)
{
    b->store_kv(self_name, datum);
}

namespace options {
namespace detail {

void set_base_value_to_default(value_* val)
{
    val->set_to_default();
}

} // ns options::detail
} // ns options

