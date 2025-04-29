#include "ftnoir_protocol_wine.h"
#include <qprocess.h>
#ifndef OTR_WINE_NO_WRAPPER
#   include "csv/csv.h"
#endif

#include <cstring>
#include <cmath>

#include <QString>
#include <QDebug>

#include "proton.h"

wine::wine() = default;

wine::~wine()
{
#ifndef OTR_WINE_NO_WRAPPER
    bool exit = false;
    if (shm) {
        shm->stop = true;
        exit = wrapper.waitForFinished(100);
        if (exit)
            qDebug() << "proto/wine: wrapper exit code" << wrapper.exitCode();
    }
    if (!exit)
    {
        if (wrapper.state() != QProcess::NotRunning)
            wrapper.kill();
        wrapper.waitForFinished(1000);
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
            //qDebug() << "proto/wine: looking up gameData";
            QString gamename;
            QMutexLocker foo(&game_name_mutex);
            /* only EZCA for FSX requires dummy process, and FSX doesn't work on Linux */
            /* memory-hacks DLL can't be loaded into a Linux process, either */
            getGameData(shm->gameid, shm->table, gamename);
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

    /////////////////////////
    // determine wine path //
    /////////////////////////
    QString wine_path = "wine";

    if (s.variant_wine) {
        // NORMAL WINE

        // resolve combo box
        if (s.wine_select_path().toString() != "WINE") {
            // if we are not supposed to use system wine then:
            if (s.wine_select_path().toString() != "CUSTOM") {
                // if we don't have a custom path then change the wine_path to the path corresponding to the selected version
                wine_path = s.wine_select_path().toString();
            }
            else if (!s.wine_custom_path->isEmpty()) {
                // if we do have a custom path and it is not empty then
                wine_path = s.wine_custom_path;
            }
        }

        // parse tilde if present
        if (wine_path[0] == '~')
            wine_path = qgetenv("HOME") + wine_path.mid(1);
    }
    else if (s.variant_proton)
    {
        // PROTON

        wine_path = s.proton_path().toString() + "/bin/wine";
    }
    qDebug() << "proto/wine: wine_path:" << wine_path;


    /////////////////////////////////////
    // determine environment variables //
    /////////////////////////////////////
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

    // if proton is used setup proton environment
    if (s.variant_proton)
    {
        auto [proton_env, env_error_string, env_success] = make_steam_environ(s.proton_path().toString());
        env = proton_env;

        if (!env_success)
            return error(env_error_string);
    }

    // determine wineprefix
    if (s.variant_proton && s.variant_proton_steamplay) {
        // wine prefix is dependend on steam

        if (s.proton_appid == 0)
            return error(tr("Must specify application id for Proton (Steam Play)"));

        auto [prefix, error_string, success] = make_wineprefix(s.proton_appid);
        qDebug() << "proto/wine: wineprefix:" << prefix;
        env.insert("WINEPREFIX", prefix);

        if (!success)
            return error(error_string);
    }
    else {
        // wine prefix was supplied via path

        QString wineprefix = "";

        // check if prefix was supplied via wine
        if (s.variant_wine && !s.wineprefix->isEmpty())
            wineprefix = s.wineprefix;

        // check if prefix was supplied via proton
        if (s.variant_proton_external && !s.protonprefix->isEmpty())
            wineprefix = s.protonprefix;

        // check if the user specified a prefix anywhere
        if (wineprefix.isEmpty())
            return error(tr("Prefix has not been defined!").arg(wineprefix));

        // handle tilde
        if (wineprefix[0] == '~')
            wineprefix = qgetenv("HOME") + wineprefix.mid(1);

        // return error if relative path is given
        if (wineprefix[0] != '/')
            return error(tr("Wine prefix must be an absolute path (given '%1')").arg(wineprefix));

        qDebug() << "proto/wine: wineprefix:" << wineprefix;

        env.insert("WINEPREFIX", wineprefix);
    }

    // ESYNC and FSYNC
    if (s.esync)
        env.insert("WINEESYNC", "1");
    if (s.fsync)
        env.insert("WINEFSYNC", "1");

    // Headtracking Protocol
    env.insert("OTR_WINE_PROTO", QString::number(s.protocol+1));


    ////////////////////////////////
    // launch the wrapper program //
    ////////////////////////////////

    wrapper.setProcessEnvironment(env);
    wrapper.setWorkingDirectory(OPENTRACK_BASE_PATH);
    wrapper.start(wine_path, { library_path + "opentrack-wrapper-wine.exe.so" });
    wrapper.waitForStarted();
    if (wrapper.state() == QProcess::ProcessState::NotRunning) {
        return error(tr("Failed to start Wine! Make sure the binary is set correctly."));
    }
#endif

    if (lck_shm.success())
    {
        shm = (WineSHM*) lck_shm.ptr();
        memset(shm, 0, sizeof(*shm));

        qDebug() << "proto/wine: shm success";

        // display "waiting for game message" (overwritten once a game is detected)
#ifndef OTR_WINE_NO_WRAPPER
        connected_game = "waiting for game...";
#endif
    }
    else {
        qDebug() << "proto/wine: shm no success";
    }

    if (lck_shm.success())
        return status_ok();
    else
        return error(tr("Can't open shared memory mapping"));
}

OPENTRACK_DECLARE_PROTOCOL(wine, FTControls, wine_metadata)
