/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2013	Wim Vriend (Developing)									*
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
*				to games, using the FreeTrackClient.dll.	         			*
********************************************************************************/
#include "ftnoir_protocol_ft.h"
#include "ftnoir_csv/csv.h"

FTNoIR_Protocol::FTNoIR_Protocol() :
    shm(FT_MM_DATA, FREETRACK_MUTEX, sizeof(FTMemMap))
{
    pMemData = (FTMemMap*) shm.mem;
    ProgramName = "";
    intGameID = 0;
    viewsStart = 0;
    viewsStop = 0;
}

FTNoIR_Protocol::~FTNoIR_Protocol()
{
    if (viewsStop != NULL) {
        viewsStop();
        FTIRViewsLib.unload();
    }
    dummyTrackIR.terminate();
    dummyTrackIR.kill();
    dummyTrackIR.waitForFinished(50);
}

void FTNoIR_Protocol::sendHeadposeToGame(const double* headpose) {
    float virtPosX;
    float virtPosY;
    float virtPosZ;

    float virtRotX;
    float virtRotY;
    float virtRotZ;

    float headPosX;
    float headPosY;
    float headPosZ;

    float headRotX;
    float headRotY;
    float headRotZ;
    headRotX = virtRotX = getRadsFromDegrees(headpose[Pitch]) * (s.useDummyExe ? 2.0 : 1.0);
    headRotY = virtRotY = getRadsFromDegrees(headpose[Yaw]);
    headRotZ = virtRotZ = getRadsFromDegrees(headpose[Roll]);
    headPosX = virtPosX = headpose[TX] * 10;
    headPosY = virtPosY = headpose[TY] * 10;
    headPosZ = virtPosZ = headpose[TZ] * 10;

    shm.lock();
    
    pMemData->data.RawX = headPosX;
    pMemData->data.RawY = headPosY;
    pMemData->data.RawZ = headPosZ;
    pMemData->data.RawPitch = headRotX;
    pMemData->data.RawYaw = headRotY;
    pMemData->data.RawRoll = headRotZ;

    //
    //
    pMemData->data.X = virtPosX;
    pMemData->data.Y = virtPosY;
    pMemData->data.Z = virtPosZ;
    pMemData->data.Pitch = virtRotX;
    pMemData->data.Yaw = virtRotY;
    pMemData->data.Roll = virtRotZ;

    //
    // Leave some values 0 yet...
    //
    pMemData->data.X1 = pMemData->data.DataID + 10;
    pMemData->data.X2 = 0;
    pMemData->data.X3 = 0;
    pMemData->data.X4 = 0;
    pMemData->data.Y1 = 0;
    pMemData->data.Y2 = 0;
    pMemData->data.Y3 = 0;
    pMemData->data.Y4 = 0;

    if (intGameID != pMemData->GameID)
    {
        QString gamename;
        CSV::getGameData(pMemData->GameID, pMemData->table, gamename);
        pMemData->GameID2 = pMemData->GameID;
        intGameID = pMemData->GameID;
        QMutexLocker foo(&this->game_name_mutex);
        connected_game = gamename;
    }

	pMemData->data.DataID += 1;
        shm.unlock();
}

void FTNoIR_Protocol::start_tirviews() {
    QString aFileName = QCoreApplication::applicationDirPath() + "/TIRViews.dll";
    if ( QFile::exists( aFileName )) {
        FTIRViewsLib.setFileName(aFileName);
        FTIRViewsLib.load();

        viewsStart = (importTIRViewsStart) FTIRViewsLib.resolve("TIRViewsStart");
        if (viewsStart == NULL) {
            qDebug() << "FTServer::run() says: TIRViewsStart function not found in DLL!";
        }
        else {
            qDebug() << "FTServer::run() says: TIRViewsStart executed!";
            viewsStart();
        }

        //
        // Load the Stop function from TIRViews.dll. Call it when terminating the thread.
        //
        viewsStop = (importTIRViewsStop) FTIRViewsLib.resolve("TIRViewsStop");
        if (viewsStop == NULL) {
            qDebug() << "FTServer::run() says: TIRViewsStop function not found in DLL!";
        }
    }
}

void FTNoIR_Protocol::start_dummy() {


    QString program = QCoreApplication::applicationDirPath() + "/TrackIR.exe";
    dummyTrackIR.setProgram("\"" + program + "\"");
    dummyTrackIR.start();

    qDebug() << "FTServer::run() says: TrackIR.exe executed!" << program;
}

bool FTNoIR_Protocol::checkServerInstallationOK()
{   
	QSettings settings("Freetrack", "FreetrackClient");							// Registry settings (in HK_USER)
	QSettings settingsTIR("NaturalPoint", "NATURALPOINT\\NPClient Location");	// Registry settings (in HK_USER)
	QString aLocation;															// Location of Client DLL


    if (!shm.success())
        return false;

	qDebug() << "checkServerInstallationOK says: Starting Function";

    //
    // Write the path in the registry (for FreeTrack and FreeTrack20), for the game(s).
    //
    aLocation =  QCoreApplication::applicationDirPath() + "/";

    qDebug() << "checkServerInstallationOK says: used interface = " << s.intUsedInterface;
    switch (s.intUsedInterface) {
        case 0:									// Use both interfaces
            settings.setValue( "Path" , aLocation );
            settingsTIR.setValue( "Path" , aLocation );
            break;
        case 1:									// Use FreeTrack, disable TrackIR
            settings.setValue( "Path" , aLocation );
            settingsTIR.setValue( "Path" , "" );
            break;
        case 2:									// Use TrackIR, disable FreeTrack
            settings.setValue( "Path" , "" );
            settingsTIR.setValue( "Path" , aLocation );
            break;
        default:
            // should never be reached
        break;
    }

    //
    // TIRViews must be started first, or the NPClient DLL will never be loaded.
    //
    if (s.useTIRViews) {
        start_tirviews();
    }

    // more games need the dummy executable than previously thought
    start_dummy();

    pMemData->data.DataID = 1;
    pMemData->data.CamWidth = 100;
    pMemData->data.CamHeight = 250;
    pMemData->GameID2 = 0;
    memset(pMemData->table, 0, 8);
    
	return true;
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocol* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Protocol;
}
