#include "scoped.hpp"

namespace options {

opts::~opts()
{
    b->reload();
}

opts::opts(const QString &name) : b(make_bundle(name))
{
}

}
