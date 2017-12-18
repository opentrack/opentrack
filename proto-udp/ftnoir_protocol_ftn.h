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

#include "ui_ftnoir_ftncontrols.h"
#include <QUdpSocket>
#include "api/plugin-api.hpp"
#include "options/options.hpp"
using namespace options;

struct settings : opts {
    value<int> ip1, ip2, ip3, ip4, port;
    settings() :
        opts("udp-proto"),
        ip1(b, "ip1", 192),
        ip2(b, "ip2", 168),
        ip3(b, "ip3", 0),
        ip4(b, "ip4", 2),
        port(b, "port", 4242)
    {}
};

class udp : public QObject, public IProtocol
{
    Q_OBJECT

public:
    udp();
    module_status initialize() override;
    void pose(const double *headpose);
    QString game_name() {
        return otr_tr("UDP over network");
    }
private:
    QUdpSocket outSocket;
    settings s;

    QHostAddress dest_ip { 127u << 24 | 1u };
    unsigned short dest_port = 65535;

private slots:
    void set_dest_address();
};

// Widget that has controls for FTNoIR protocol client-settings.
class FTNControls: public IProtocolDialog
{
    Q_OBJECT

public:
    FTNControls();
    void register_protocol(IProtocol *) {}
    void unregister_protocol() {}
private:
    Ui::UICFTNControls ui;
    settings s;
private slots:
    void doOK();
    void doCancel();
};

class udpDll : public Metadata
{
public:
    QString name() { return otr_tr("UDP over network"); }
    QIcon icon() { return QIcon(":/images/facetracknoir.png"); }
};
