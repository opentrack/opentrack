#include "tr.hpp"


TR::TR() : QObject(nullptr) {}

TR::TR(const TR&) : QObject(nullptr) {}

TR& TR::operator=(const TR& other)
{
    if (this == &other)
        return *this;

    this->~TR();
    return *new (this) TR;
}

TR::~TR() {}
