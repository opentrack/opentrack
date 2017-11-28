/* Copyright (c) 2017, Eike Ziller                                               *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */

#pragma once

#include "api/plugin-api.hpp"

class IOKitProtocolDialog : public IProtocolDialog
{
    Q_OBJECT

public:
    IOKitProtocolDialog();

    void register_protocol(IProtocol *protocol) final;
    void unregister_protocol() final;
};
