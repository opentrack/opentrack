#include "ftnoir_protocol_wine.h"
#include <QString>
#include <string.h>
#include <math.h>
#ifndef OTR_WINE_NO_WRAPPER
#   include "csv/csv.h"
#endif
#include "compat/library-path.hpp"

wine::wine() = default;

wine::~wine()
{
#ifndef OTR_WINE_NO_WRAPPER
    bool exit = false;
    if (shm) {
        shm->stop = true;
        exit = wrapper.waitForFinished(100);
    }
    if (!exit)
    {
        wrapper.kill();
        wrapper.waitForFinished(-1);
    }
#endif
    //shm_unlink("/" WINE_SHM_NAME);
}

void wine::pose(const double *headpose, const double*)
{
    if (shm)
    {
        lck_shm.lock();
        for (int i = 3; i < 6; i++)
            shm->data[i] = (headpose[i] * M_PI) / 180;
        for (int i = 0; i < 3; i++)
            shm->data[i] = headpose[i] * 10;
#ifndef OTR_WINE_NO_WRAPPER
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
#endif
        lck_shm.unlock();
    }
}

module_status wine::initialize()
{
#ifndef OTR_WINE_NO_WRAPPER
    static const QString library_path(OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH);

    QString wine_path = "wine";
    auto env = QProcessEnvironment::systemEnvironment();

    if (s.variant_proton)
    {
        if (s.proton_appid == 0)
            return error(tr("Must specify application id for Proton (Steam Play)"));

        QProcessEnvironment make_steam_environ(const QString& proton_version, int appid);
        QString proton_path(const QString& proton_version);

        wine_path = proton_path(s.proton_version);
        env = make_steam_environ(s.proton_version, s.proton_appid);
    }
    else
    {
        QString wineprefix = "~/.wine";
        if (!s.wineprefix->isEmpty())
            wineprefix = s.wineprefix;
        if (wineprefix[0] == '~')
            wineprefix = qgetenv("HOME") + wineprefix.mid(1);

        if (wineprefix[0] != '/')
            error(tr("Wine prefix must be an absolute path (given '%1')").arg(wineprefix));

        env.insert("WINEPREFIX", wineprefix);
    }

    if (s.esync)
        env.insert("WINEESYNC", "1");
    if (s.fsync)
        env.insert("WINEFSYNC", "1");

    wrapper.setProcessEnvironment(env);
    wrapper.setWorkingDirectory(OPENTRACK_BASE_PATH);
    wrapper.start(wine_path, { library_path + "opentrack-wrapper-wine.exe.so" });
#endif

    if (lck_shm.success())
    {
        shm = (WineSHM*) lck_shm.ptr();
        memset(shm, 0, sizeof(*shm));
    }

    if (lck_shm.success())
        return status_ok();
    else
        return error(tr("Can't open shared memory mapping"));
}

OPENTRACK_DECLARE_PROTOCOL(wine, FTControls, wine_metadata)
