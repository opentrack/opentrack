#pragma once
#if defined OTR_DBUS_CONTROL

#include "export.hpp"
#include "pipeline.hpp"

#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusError>

class OTR_LOGIC_EXPORT DBus final : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.github.opentrack.Tracker")

    Q_PROPERTY(bool IsEnabled READ is_enabled)
    Q_PROPERTY(bool IsZeroed READ is_zero)

    pipeline* pipeline_;

public:
    static constexpr const char* SERVICE_NAME = "com.github.opentrack.Tracker";
    static constexpr const char* SERVICE_PATH = "/com/github/opentrack/Tracker";

    DBus(QObject* obj, pipeline* pipeline)
        : QDBusAbstractAdaptor(obj)
        , pipeline_(pipeline)
    { }
    ~DBus() = default;

    bool is_enabled() const;
    bool is_zero() const;

public slots:
    Q_SCRIPTABLE Q_NOREPLY void Center();
    Q_SCRIPTABLE Q_NOREPLY void ToggleEnabled();
    Q_SCRIPTABLE Q_NOREPLY void ToggleZero();
};

#endif
