#include "ftnoir_protocol_wine.h"
#include "opentrack-library-path.h"
#include <QString>
#include <QStringList>
#include <QCoreApplication>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include "csv/csv.h"

wine::wine() : lck_shm(WINE_SHM_NAME, WINE_MTX_NAME, sizeof(WineSHM)), shm(NULL), gameid(0)
{
    if (lck_shm.success()) {
        shm = (WineSHM*) lck_shm.ptr();
        memset(shm, 0, sizeof(*shm));
    }
    static const QString library_path(QCoreApplication::applicationDirPath() + OPENTRACK_LIBRARY_PATH);
    wrapper.setWorkingDirectory(QCoreApplication::applicationDirPath());
    wrapper.start("wine", QStringList() << (library_path + "opentrack-wrapper-wine.exe.so"));
}

wine::~wine()
{
    if (shm) {
        shm->stop = true;
        wrapper.waitForFinished(100);
    }
    wrapper.close();
    //shm_unlink("/" WINE_SHM_NAME);
}

void wine::pose( const double *headpose )
{
    if (shm)
    {
        lck_shm.lock();
        for (int i = 3; i < 6; i++)
            shm->data[i] = headpose[i] / (180 / M_PI );
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

bool wine::correct()
{
    return lck_shm.success();
}

OPENTRACK_DECLARE_PROTOCOL(wine, FTControls, wineDll)
