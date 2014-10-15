/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once
#include "ui_ftnoir_libevdev_controls.h"

#include <QMessageBox>
#include "facetracknoir/plugin-api.hpp"

extern "C" {
#   include <libevdev/libevdev.h>
#   include <libevdev/libevdev-uinput.h>
}

class FTNoIR_Protocol : public IProtocol
{
public:
	FTNoIR_Protocol();
    ~FTNoIR_Protocol() override;
    bool checkServerInstallationOK() {
        return dev != NULL;
    }
    void sendHeadposeToGame(const double *headpose);
    QString getGameName() {
        return "Virtual joystick for Linux";
    }
private:
    struct libevdev* dev;
    struct libevdev_uinput* uidev;
};

class LibevdevControls: public QWidget, public IProtocolDialog
{
    Q_OBJECT
public:

    explicit LibevdevControls();
    void registerProtocol(IProtocol *) {}
	void unRegisterProtocol() {}

private:
    Ui::UICLibevdevControls ui;
	void save();

private slots:
	void doOK();
	void doCancel();
};

//*******************************************************************************************************
// FaceTrackNoIR Protocol DLL. Functions used to get general info on the Protocol
//*******************************************************************************************************
class FTNoIR_ProtocolDll : public Metadata
{
public:
	FTNoIR_ProtocolDll();
	~FTNoIR_ProtocolDll();

    void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("libevdev"); }
    void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("libevdev"); }
    void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("libevdev"); }

    void getIcon(QIcon *icon) { *icon = QIcon(":/images/linux.png"); }
};
