/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*																				*
* FTServer		FTServer is the Class, that communicates headpose-data			*
*				to games, using the FreeTrackClient.dll.		         		*
********************************************************************************/
#pragma once
#include "ui_ftnoir_ftcontrols.h"
#include "facetracknoir/plugin-api.hpp"
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QMutex>
#include <QMutexLocker>
#include "compat/compat.h"
#include "facetracknoir/options.h"
#include "fttypes.h"
using namespace options;

struct settings {
    pbundle b;
    value<int> intUsedInterface;
    value<bool> useTIRViews;
    settings() :
        b(bundle("proto-freetrack")),
        intUsedInterface(b, "used-interfaces", 0),
        useTIRViews(b, "use-memory-hacks", false)
    {}
};

typedef void (__stdcall *importTIRViewsStart)(void);
typedef void (__stdcall *importTIRViewsStop)(void);

class FTNoIR_Protocol : public IProtocol
{
public:
    FTNoIR_Protocol();
    ~FTNoIR_Protocol() override;
    bool correct(  );
    void pose( const double *headpose );
    QString game_name() override {
        QMutexLocker foo(&game_name_mutex);
        return connected_game;
    }
private:
    settings s;
    FTHeap *pMemData;
    PortableLockedShm shm;

    QLibrary FTIRViewsLib;
    QProcess dummyTrackIR;
    importTIRViewsStart viewsStart;
    importTIRViewsStop viewsStop;    
    
    int intGameID;
    QString connected_game;
    QMutex game_name_mutex;
    
    static inline double getRadsFromDegrees(double degrees) { return degrees * 0.017453; }
    void start_tirviews();
    void start_dummy();
};

class FTControls: public IProtocolDialog
{
    Q_OBJECT
public:
    FTControls();
    void register_protocol(IProtocol *) {}
    void unregister_protocol() {}
private:
    Ui::UICFTControls ui;
    settings s;
private slots:
    void selectDLL();
    void doOK();
    void doCancel();
};

class FTNoIR_ProtocolDll : public Metadata
{
public:
    QString name() { return QString("freetrack 2.0 Enhanced"); }
    QIcon icon() { return QIcon(":/images/freetrack.png"); }
};
