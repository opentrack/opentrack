#pragma once

#include <QObject>

// The class does nothing except provide a fake assignment operator for QObject
// It's meant to be used inside classes that need i18n support but are returned by value.

struct TR : QObject
{
    TR() = default;
    TR(const TR&) : QObject(nullptr) {}

    TR& operator=(const TR&) { return *this; }
};
