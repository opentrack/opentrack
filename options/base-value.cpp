#include "base-value.hpp"

using namespace options;

base_value::base_value(bundle b, const QString& name, base_value::comparator cmp, std::type_index type_idx) :
    b(b),
    self_name(name),
    cmp(cmp),
    type_index(type_idx)
{
    b->on_value_created(name, this);
}

base_value::~base_value()
{
    b->on_value_destructed(self_name, this);
}

void base_value::notify() const
{
    bundle_value_changed();
}

void base_value::store(const QVariant& datum)
{
    b->store_kv(self_name, datum);
}

void ::options::detail::set_base_value_to_default(base_value* val)
{
    val->set_to_default();
}
