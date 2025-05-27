#include "ftnoir_protocol_shm.h"
#include <QString>
#include <QDebug>

native_shm::native_shm() = default;

native_shm::~native_shm()
{
    //shm_unlink("/" WINE_SHM_NAME);
}

void native_shm::pose(const double *headpose, const double*)
{
    if (shm)
    {
        lck_shm.lock();
        for (int i = 3; i < 6; i++)
            shm->data[i] = (headpose[i] * M_PI) / 180;
        for (int i = 0; i < 3; i++)
            shm->data[i] = headpose[i] * 10;
        lck_shm.unlock();
    }
}

module_status native_shm::initialize()
{

    if (lck_shm.success())
    {
        shm = (WineSHM*) lck_shm.ptr();
        memset(shm, 0, sizeof(*shm));

        qDebug() << "proto/native_shm: shm success";
    }
    else {
        qDebug() << "proto/native_shm: shm no success";
    }

    if (lck_shm.success())
        return status_ok();
    else
        return error(tr("Can't open shared memory mapping"));
}

OPENTRACK_DECLARE_PROTOCOL(native_shm, FTControls, wine_metadata)
