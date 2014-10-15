#include "ftnoir_protocol_wine.h"
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include "ftnoir_csv/csv.h"

FTNoIR_Protocol::FTNoIR_Protocol() : lck_shm(WINE_SHM_NAME, WINE_MTX_NAME, sizeof(WineSHM)), shm(NULL), gameid(0)
{
    if (lck_shm.success()) {
        shm = (WineSHM*) lck_shm.ptr();
        memset(shm, 0, sizeof(*shm));
    }
    wrapper.start("wine", QStringList() << (QCoreApplication::applicationDirPath() + "/opentrack-wrapper-wine.exe.so"));
}

FTNoIR_Protocol::~FTNoIR_Protocol()
{
    if (shm) {
        shm->stop = true;
        wrapper.waitForFinished(100);
    }
    wrapper.terminate();
    if (!wrapper.waitForFinished(100))
    {
        wrapper.kill();
        wrapper.waitForFinished(42);
    }
    //shm_unlink("/" WINE_SHM_NAME);
}

void FTNoIR_Protocol::sendHeadposeToGame( const double *headpose ) {
    if (shm)
    {
        lck_shm.lock();
        for (int i = 3; i < 6; i++)
            shm->data[i] = headpose[i] / 57.295781;
        for (int i = 0; i < 3; i++)
            shm->data[i] = headpose[i] * 10;
        if (shm->gameid != gameid)
        {
            QString gamename;
            QMutexLocker foo(&game_name_mutex);
            /* only EZCA for FSX requires dummy process, and FSX doesn't work on Linux */
            /* memory-hacks DLL can't be loaded into a Linux process, either */
            CSV::getGameData(shm->gameid, shm->table, gamename);
            gameid = shm->gameid2 = shm->gameid;
            connected_game = gamename;
        }
        lck_shm.unlock();
    }
}

bool FTNoIR_Protocol::checkServerInstallationOK()
{
    return lck_shm.success();
}

extern "C" OPENTRACK_EXPORT void* GetConstructor()
{
    return (IProtocol*) new FTNoIR_Protocol;
}
