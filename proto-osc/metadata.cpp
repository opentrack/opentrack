#include "metadata.hpp"
#include "proto.hpp"
#include "dialog.hpp"

QString osc_metadata::name() { return tr("Open Sound Control"); }
QIcon osc_metadata::icon() { return QIcon(":/images/osc-icon.png"); }

OPENTRACK_DECLARE_PROTOCOL(osc_proto, osc_dialog, osc_metadata)
