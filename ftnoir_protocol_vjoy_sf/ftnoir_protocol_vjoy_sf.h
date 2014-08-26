#pragma once
#include "stdafx.h"
#include "public.h"
#include "vjoyinterface.h"
#include "ftnoir_protocol_base/ftnoir_protocol_base.h"
#include "ui_ftnoir_vjoy_sf_controls.h"
#include "facetracknoir/plugin-support.h"
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QString>
#include <windows.h>
#include <QMutex>
#include <QMutexLocker>
#include "compat/compat.h"
#include "facetracknoir/options.h"
using namespace options;

struct settings {
    pbundle b;
    value<int> intvJoyID;
    // TODO: option with mapping
    // value<bool> hasXAxis, hasYAxis, hasZAxis, hasRXAxis, hasRYAxis, hasRZAxis;
    settings() :
        b(bundle("proto-vjoy-sf")),
        intvJoyID(b, "vjoy-id", 1)
    {}
};

class FTNoIR_Protocol : public IProtocol
{
public:
    BOOL hasAxisX;
    BOOL hasAxisY;
    BOOL hasAxisZ;
    BOOL hasAxisRX;
    BOOL hasAxisRY;
    BOOL hasAxisRZ;

    FTNoIR_Protocol();
    virtual ~FTNoIR_Protocol();
    bool checkServerInstallationOK();
    void sendHeadposeToGame( const double *headpose );
    QString getGameName() {
        return "Virtual joystick (Sourceforge)";
    }
private:
    int      intvJoyID;
    settings s;
    JOYSTICK_POSITION_V2 vJoyPosition;					// The structure that holds the full position data
    LONG lAxesMax[6], lAxesMin[6];      // max and min logical value of every axis
    BYTE bytevJoyID; // have no idea why it use weird way like that but since it's in the official docs...
};

// Widget that has controls for FTNoIR protocol client-settings.
class VJoySFControls: public QWidget, public IProtocolDialog
{
    Q_OBJECT
public:

    explicit VJoySFControls();
    void registerProtocol(IProtocol *l) {}
    void unRegisterProtocol() {}

private:
    Ui::UICVJoySFControls ui;
    void save();

private slots:
    void doOK();
    void doCancel();
};

class FTNoIR_ProtocolDll : public Metadata
{
public:
    void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("vJoy SF"); }
    void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("vJoy (Sourceforge)"); }
    void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("vJoy - Virtual Joystick, sourceforge version: http://vjoystick.sourceforge.net"); }
    void getIcon(QIcon *icon) { *icon = QIcon(":/images/vjoy-sf.png"); }
};
