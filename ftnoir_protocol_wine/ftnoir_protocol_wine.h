#pragma once

#include "ftnoir_protocol_base/ftnoir_protocol_base.h"
#include "ftnoir_protocol_ft/fttypes.h"
#include "ftnoir_csv/csv.h"
#include "ui_ftnoir_winecontrols.h"
#include <QMessageBox>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <QFile>
#include "facetracknoir/plugin-support.h"
#include "compat/compat.h"
#include "ftnoir_protocol_wine/wine-shm.h"

class FTNoIR_Protocol : public IProtocol
{
public:
	FTNoIR_Protocol();
    virtual ~FTNoIR_Protocol();

    bool checkServerInstallationOK();
    void sendHeadposeToGame(const double* headpose);
    QString getGameName() {
        QMutexLocker foo(&game_name_mutex);
        return connected_game;
    }
private:
    PortableLockedShm lck_shm;
    WineSHM* shm;
    QProcess wrapper;
    int gameid;
    QString connected_game;
    QMutex game_name_mutex;
};

class FTControls: public QWidget, public IProtocolDialog
{
    Q_OBJECT
public:
    FTControls();
    void registerProtocol(IProtocol *) {}
    void unRegisterProtocol() {}

private:
    Ui::UICFTControls ui;

private slots:
	void doOK();
	void doCancel();
};

class FTNoIR_ProtocolDll : public Metadata
{
public:
	FTNoIR_ProtocolDll();
	~FTNoIR_ProtocolDll();

    void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("Wine"); }
    void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("Wine"); }
    void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("Wine glue wrapper"); }

    void getIcon(QIcon *icon) { *icon = QIcon(":/images/wine.png"); }
};
