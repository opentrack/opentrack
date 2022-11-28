#include <cerrno>
#include <cmath>
#include <cstring>
#include <string>

// OSX sdk 10.8 build error otherwise
#undef _LIBCPP_MSVCRT
#include <cstdio>

#include "freetrackclient/fttypes.h"
#include "compat/export.hpp"

enum Axis {
    TX = 0, TY, TZ, Yaw, Pitch, Roll
};

#define __WINE_OLE2_H
#include "compat/shm.h"
#include "wine-shm.h"

void create_registry_key(void);

class ShmPosix {
public:
    ShmPosix(const char *shmName, const char *mutexName, int mapSize);
    ~ShmPosix();
    void lock();
    void unlock();
    bool success();
    inline void* ptr() { return mem; }
private:
    void* mem;
    int fd, size;
};

class ShmWine {
public:
    ShmWine(const char *shmName, const char *mutexName, int mapSize);
    ~ShmWine();
    void lock();
    void unlock();
    bool success();
    inline void* ptr() { return mem; }
private:
    void* mem;
    void *hMutex, *hMapFile;
};

int main(void)
{
    ShmPosix lck_posix(WINE_SHM_NAME, WINE_MTX_NAME, sizeof(WineSHM));
    ShmWine lck_wine("FT_SharedMem", "FT_Mutext", sizeof(FTHeap));
    if(!lck_posix.success()) {
        fprintf(stderr, "Can't open posix map: %d\n", errno);
        return 1;
    }
    if(!lck_wine.success()) {
        fprintf(stderr, "Can't open Wine map\n");
        return 1;
    }
    WineSHM* shm_posix = (WineSHM*) lck_posix.ptr();
    FTHeap* shm_wine = (FTHeap*) lck_wine.ptr();
    FTData* data = &shm_wine->data;
    //strcpy(shm_posix->message, "SHM SET UP AND STARTING MAIN LOOP");
    create_registry_key(); // FORK: EDIT FOR SC. FreeTrack Key has been removed. See function for details.
    while (true) {
        if (shm_posix->stop)
            break;
        data->Yaw = -shm_posix->data[Yaw];
        data->Pitch = -shm_posix->data[Pitch];
        data->Roll = shm_posix->data[Roll];
        data->X = shm_posix->data[TX];
        data->Y = shm_posix->data[TY];
        data->Z = shm_posix->data[TZ];
        data->DataID++;
        data->CamWidth = 250;
        data->CamHeight = 100;
        shm_wine->GameID2 = shm_posix->gameid2;
        shm_posix->gameid = shm_wine->GameID;
        for (int i = 0; i < 8; i++)
            shm_wine->table[i] = shm_posix->table[i];
        //strcpy(shm_posix->message, std::string(std::to_string(shm_wine->GameID) + " : " + std::to_string(shm_wine->GameID2)).c_str());
        (void) Sleep(4);
    }
}
