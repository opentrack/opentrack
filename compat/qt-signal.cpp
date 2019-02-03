#include "qt-signal.hpp"

namespace qt_sig {

nullary::nullary(QObject* parent) : QObject(parent) {}
nullary::~nullary() = default;

void nullary::operator()() const
{
    notify();
}

} // ns qt_sig
