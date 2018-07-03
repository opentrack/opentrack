#include "tr.hpp"


TR::TR() : QObject(nullptr) {}

TR::TR(const TR&) : QObject(nullptr) {}

TR& TR::operator=(const TR&)
{
    return *this;
}

