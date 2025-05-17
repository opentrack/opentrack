#include "ftnoir_tracker_linux_joystick.h"

#include <QDir>
#include <QFileInfo>
#include <QVariant>
#include <QRegularExpression>

// Discovery is done by searching for devices in the sys file system.
//
// Given a path like this
// /sys/devices/pci0000:00/0000:00:14.0/usb1/1-10/1-10:1.2/0003:2341:8036.0170/input/input380/js0
// we want to get this part of the string 2341:8036, it will allow us to
// identify the device in the future.
// alternative way of doing this https://stackoverflow.com/questions/21173988/linux-attempting-to-get-joystick-vendor-and-product-ids-via-ioctl-get-einval-i
std::tuple<QString, QString> sysfsDeviceToJsDev(QFileInfo device) {
    using ret = std::tuple<QString, QString>;
    QString symlink = device.symLinkTarget();
    QString js_dev = QString("/dev/input/%1").arg(device.fileName());

    QRegularExpression sep(QString("[:.%1]").arg(QDir::separator()));
    QString device_id = symlink.section(sep, -6, -5);
    return ret(js_dev, device_id);
}

QList<linux_joystick> getJoysticks()
{
    char name[128];
    QList<linux_joystick> joysticks;

    QDir dir("/sys/class/input/");
    dir.setNameFilters({ "js*" });
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo device = list.at(i);
        auto [js_dev, device_id] = sysfsDeviceToJsDev(device);
        int iFile = open(js_dev.toUtf8().data(), O_RDONLY | O_NONBLOCK);
        if (iFile == -1) continue;
        if (ioctl(iFile, JSIOCGNAME(sizeof(name)), &name) > 0)
        {
            linux_joystick j;
            j.name = name;
            j.dev = js_dev;
            j.device_id = device_id;
            joysticks.append(j);
        }
        close(iFile);

    }

    return joysticks;
}

QString getJoystickDevice(QString guid) {
    QDir dir("/sys/class/input/");
    dir.setNameFilters({ "js*" });
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo device = list.at(i);
        auto [js_dev, device_id] = sysfsDeviceToJsDev(device);
        if (device_id == guid) return js_dev;
    }

    return {};
}
