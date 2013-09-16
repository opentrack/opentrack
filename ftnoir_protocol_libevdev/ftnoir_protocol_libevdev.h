/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once
#include "ftnoir_protocol_base/ftnoir_protocol_base.h"
#include "ui_ftnoir_libevdev_controls.h"

#include <QMessageBox>
#include <QSettings>
#include "facetracknoir/global-settings.h"

#include <libevdev-1.0/libevdev/libevdev.h>
#include <libevdev-1.0/libevdev/libevdev-uinput.h>

class FTNoIR_Protocol : public IProtocol
{
public:
	FTNoIR_Protocol();
	~FTNoIR_Protocol();
    bool checkServerInstallationOK() {
        return dev != NULL;
    }
    void sendHeadposeToGame( double *headpose, double *rawheadpose );
    QString getGameName() {
        return "Virtual joystick for Linux";
    }
private:
    struct libevdev* dev;
    struct libevdev_uinput* uidev;
};

// Widget that has controls for FTNoIR protocol client-settings.
class LibevdevControls: public QWidget, public IProtocolDialog
{
    Q_OBJECT
public:

    explicit LibevdevControls();
    virtual ~LibevdevControls();
    void showEvent ( QShowEvent *) {}

    void Initialize(QWidget *);
    void registerProtocol(IProtocol *l) {}
	void unRegisterProtocol() {}

private:
	Ui::UICVJoyControls ui;
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
