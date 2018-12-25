#include <cerrno>
#include <unistd.h> // usleep

#include "compat/macros1.h"
#include "freetrackclient/fttypes.h"
#include "wine-shm.h"

enum Axis {
    TX = 0, TY, TZ, Yaw, Pitch, Roll
};

#undef SHM_HEADER_GUARD
#undef SHMXX_HEADER_GUARD
#undef SHM_TYPE_NAME
#undef SHM_FUN_PREFIX
#undef SHMXX_TYPE_NAME
#undef SHM_WIN32_INIT
#define SHM_TYPE_NAME shm_impl_winelib
#define SHM_FUN_PREFIX shm_impl_winelib_
#define SHMXX_TYPE_NAME mem_winelib
#define SHM_WIN32_INIT 1
#include "compat/shm.hpp"

#undef SHM_HEADER_GUARD
#undef SHMXX_HEADER_GUARD
#undef SHM_TYPE_NAME
#undef SHM_FUN_PREFIX
#undef SHMXX_TYPE_NAME
#undef SHM_WIN32_INIT
#define SHM_TYPE_NAME shm_impl_unix
#define SHM_FUN_PREFIX shm_impl_unix_
#define SHMXX_TYPE_NAME mem_unix
#define SHM_WIN32_INIT
#include "compat/shm.hpp"

void create_registry_key(void);

int main(void)
{
    mem_unix lck_unix(WINE_SHM_NAME, WINE_MTX_NAME, sizeof(WineSHM));
    mem_winelib lck_wine("FT_SharedMem", "FT_Mutext", sizeof(FTHeap));

    if(!lck_unix.success())
        return 1;
    if(!lck_wine.success())
        return 1;

    create_registry_key();

    WineSHM& mem_unix = *(WineSHM*) lck_unix.ptr();
    FTHeap& mem_wine = *(FTHeap*) lck_wine.ptr();
    FTData& data = mem_wine.data;

    data.CamWidth = 250;
    data.CamHeight = 100;

    while (!mem_unix.stop)
    {
        MEMBAR();
        data.Yaw = -mem_unix.data[Yaw];
        data.Pitch = -mem_unix.data[Pitch];
        data.Roll = mem_unix.data[Roll];
        data.X = mem_unix.data[TX];
        data.Y = mem_unix.data[TY];
        data.Z = mem_unix.data[TZ];
        data.DataID = 1;
        mem_wine.GameID2 = mem_unix.gameid2;
        mem_unix.gameid = mem_wine.GameID;
        for (int i = 0; i < 8; i++)
            mem_wine.table[i] = mem_wine.table[i];
        MEMBAR();
        (void)usleep(4 * 1000);
    }
}
