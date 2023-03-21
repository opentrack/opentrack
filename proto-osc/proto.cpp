#include "proto.hpp"
#include "ui_dialog.h"
#include "api/plugin-api.hpp"
#include <QQuaternion>
#include <QHostAddress>
#include "osc/OscOutboundPacketStream.h"

osc_proto::osc_proto()
{
    auto reload_fn = [this] {
        dest = QHostAddress{s.address };
        port = (unsigned short)s.port;
    };
    connect(&*s.b, &bundle_::changed, this, reload_fn);
    connect(&*s.b, &bundle_::reloading, this, reload_fn);
}

void osc_proto::pose(const double* data, const double*)
{
    if (dest.isNull())
        return;

    static constexpr unsigned buffer_size = 1024;
    char buffer[buffer_size] = {};
    osc::OutboundPacketStream p{buffer, buffer_size};
    auto q = QQuaternion::fromEulerAngles((float)data[Pitch], (float)data[Yaw], (float)-data[Roll]).normalized();
    p << osc::BeginMessage("/bridge/quat") << q.scalar() << q.x() << q.y() << q.z() << osc::EndMessage;
    sock.writeDatagram(p.Data(), (int)p.Size(), dest, port);
}

module_status osc_proto::initialize()
{
    QString error;

    dest = QHostAddress{s.address };
    port = (unsigned short)s.port;

    if (dest.isNull())
    {
        error = tr("Invalid destination address '%1'").arg(s.address);
        goto fail;
    }

    if (!sock.bind())
    {
        error = tr("Error binding socket to INADDR_ANY");
        goto fail;
    }

    return status_ok();

fail:
    return { tr("%1: %2").arg(error, sock.errorString()) };
}
