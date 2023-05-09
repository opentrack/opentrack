#pragma once

/* Copyright (c) 2023 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "settings.hpp"
#include "api/plugin-api.hpp"
#include <QUdpSocket>
#include <QHostAddress>

class osc_proto : public QObject, public IProtocol
{
    Q_OBJECT

    osc_settings s;
    QUdpSocket sock;
    QHostAddress dest;
    unsigned short port = 0;

public:
    osc_proto();
    module_status initialize() override;
    void pose(const double *headpose, const double*) override;
    QString game_name() override { return tr("Open Sound Control"); }
};
