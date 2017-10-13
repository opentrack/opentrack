#include "falcon-bms-ext.hpp"

IExtension::event_mask falcon_bms_acceleration_ext::hook_types()
{
    return IExtension::on_finished;
}

falcon_bms_acceleration_ext::falcon_bms_acceleration_ext()
{
    qDebug() << "ext made";
}

void falcon_bms_acceleration_ext::process_finished(Pose& p)
{
}
