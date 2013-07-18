/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
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
*				to games, using the FreeTrackClient.dll.	         			*
********************************************************************************/
/*
	Modifications (last one on top):
	20110401 - WVR: Moved protocol to a DLL, convenient for installation etc.
	20101224 - WVR: Base class is no longer inheriting QThread. sendHeadposeToGame
					is called from run() of Tracker.cpp
	20100601 - WVR: Added Mutex-bit in run(). Thought it wasn't so important (still do...). 
	20100523 - WVR: Implemented the Freetrack-protocol just like Freetrack does. Earlier 
					FaceTrackNoIR only worked with an adapted DLL, with a putdata function.
					Now it works direcly in shared memory!
*/
#include "ftnoir_protocol_wine.h"
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

/** constructor **/
FTNoIR_Protocol::FTNoIR_Protocol() : lck_shm(WINE_SHM_NAME, WINE_MTX_NAME, sizeof(WineSHM)), shm(NULL), gameid(0)
{
    if (lck_shm.mem != (void*) -1) {
        shm = (WineSHM*) lck_shm.mem;
        memset(shm, 0, sizeof(*shm));
    }
}

/** destructor **/
FTNoIR_Protocol::~FTNoIR_Protocol()
{
    if (shm) {
        shm->stop = true;
        wrapper.waitForFinished(2000);
    }
    wrapper.kill();
    shm_unlink("/" WINE_SHM_NAME);
}

void FTNoIR_Protocol::Initialize()
{
    wrapper.start("wine", QStringList() << (QCoreApplication::applicationDirPath() + "/ftnoir-wrapper-wine.exe.so"));
}

void FTNoIR_Protocol::sendHeadposeToGame( double *headpose, double *rawheadpose ) {
    if (shm)
    {
        lck_shm.lock();
        for (int i = 0; i < 3; i++)
            shm->data[i] = headpose[i] / 57.295781;
        for (int i = 3; i < 6; i++)
            shm->data[i] = headpose[i] * 10;
        if (shm->gameid != gameid)
        {
            QMutexLocker foo(&game_name_mutex);
            /* only EZCA for FSX requires dummy process, and FSX doesn't work on Linux */
            /* memory-hacks DLL can't be loaded into a Linux process, either */
            CSV::getGameData(gameid, shm->table, gamename);
            gameid = shm->gameid2 = shm->gameid;
            connected_game = gamename;
        }
        lck_shm.unlock();
    }
}

//
// Check if the Client DLL exists and load it (to test it), if so.
// Returns 'true' if all seems OK.
//
bool FTNoIR_Protocol::checkServerInstallationOK()
{
    return lck_shm.mem != (void*)-1;
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Protocol object.

// Export both decorated and undecorated names.
//   GetProtocol     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetProtocol@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetProtocol=_GetProtocol@0")

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT void* CALLING_CONVENTION GetConstructor()
{
    return (IProtocol*) new FTNoIR_Protocol;
}
