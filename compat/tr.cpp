#include "tr.hpp"


TR::TR() {}

TR::TR(const TR&) {}

TR&TR::operator=(const TR& other)
{
    if (this == &other)
        return *this;

    this->~TR();
    return *new (this) TR;
}

TR::~TR() {}
