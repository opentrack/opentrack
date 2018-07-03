#pragma once

#include <QObject>
#include "export.hpp"

// The class does nothing except provide a fake assignment operator for QObject
// It's meant to be used inside classes that need i18n support but are returned by value.

class OTR_COMPAT_EXPORT TR : public QObject
{
    Q_OBJECT

public:
    TR();
    TR(const TR&);

    TR& operator=(const TR& other);
};
