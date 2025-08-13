#pragma once
#if defined OTR_DBUS_CONTROL

#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusError>

class main_window;

class MainDBus final : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.github.opentrack")

    Q_PROPERTY(bool IsTrackerRunning READ tracker_running)

    main_window* window_;

public:
    static constexpr const char* SERVICE_NAME = "com.github.opentrack";
    static constexpr const char* SERVICE_PATH = "/com/github/opentrack";

    MainDBus(QObject* obj, main_window* window)
        : QDBusAbstractAdaptor(obj)
        , window_(window)
    { }
    ~MainDBus() = default;

    bool tracker_running() const;

public slots:
    Q_SCRIPTABLE Q_NOREPLY void StartTracker();
    Q_SCRIPTABLE Q_NOREPLY void StopTracker();
    Q_SCRIPTABLE Q_NOREPLY void RestartTracker();
    Q_SCRIPTABLE Q_NOREPLY void ToggleTracker();
};

#endif
