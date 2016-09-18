/* Copyright (c) 2015-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "value.hpp"

namespace options {

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

}
