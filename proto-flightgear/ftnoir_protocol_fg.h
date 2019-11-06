/* Homepage         http://facetracknoir.sourceforge.net/home/default.htm        *
 *                                                                               *
 * ISC License (ISC)                                                             *
 *                                                                               *
 * Copyright (c) 2015, Wim Vriend                                                *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */
#pragma once
#include "ui_ftnoir_fgcontrols.h"
#include <QThread>
#include <QUdpSocket>
#include <QMessageBox>
#include "api/plugin-api.hpp"
#include "options/options.hpp"
#include "compat/tr.hpp"
using namespace options;

// x,y,z position in meters, heading, pitch and roll in degrees
#pragma pack(push, 1)
struct flightgear_datagram {
    double x, y, z, h, p, r;
    int status;
};
#pragma pack(pop)

struct settings : opts {
    value<int> ip1, ip2, ip3, ip4;
    value<int> port;
    settings() :
        opts("flightgear-proto"),
        ip1(b, "ip1", 127),
        ip2(b, "ip2", 0),
        ip3(b, "ip3", 0),
        ip4(b, "ip4", 1),
        port(b, "port", 5542)
    {}
};

class flightgear : TR, public IProtocol
{
    Q_OBJECT

public:
    void pose(const double *headpose, const double*) override;
    QString game_name() override { return tr("FlightGear"); }
    module_status initialize() override;
private:
    settings s;
    flightgear_datagram FlightData;
    QUdpSocket outSocket;
};

// Widget that has controls for FTNoIR protocol client-settings.
class FGControls: public IProtocolDialog
{
    Q_OBJECT
public:
    FGControls();
    void register_protocol(IProtocol *) override {}
    void unregister_protocol() override {}
private:
    Ui::UICFGControls ui;
    settings s;
private slots:
    void doOK();
    void doCancel();
};

class flightgearDll : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("FlightGear"); }
    QIcon icon() override { return QIcon(":/images/flightgear.png"); }
};
