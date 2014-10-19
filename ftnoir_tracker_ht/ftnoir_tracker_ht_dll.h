#include "opentrack/plugin-api.hpp"

class TrackerDll : public Metadata
{
    QString name() { return QString("ht -- face tracker"); }
    QIcon icon() { return QIcon(":/images/ht.png"); }
};
