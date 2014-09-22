#include <cerrno>
#include <cstdio>
#include "ftnoir_protocol_ft/fttypes.h"
#include "ftnoir_protocol_wine/wine-shm.h"

#define OPENTRACK_CROSS_ONLY
#include "facetracknoir/plugin-api.hpp"

#define OPENTRACK_COMPAT_BUNDLED
#include "compat/compat.h"

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
#include <windows.h>

int main(void)
{
	ShmPosix lck_posix(WINE_SHM_NAME, WINE_MTX_NAME, sizeof(WineSHM));
    ShmWine lck_wine("FT_SharedMem", "FT_Mutext", sizeof(FTHeap));
    if(!lck_posix.success()) {
		printf("Can't open posix map: %d\n", errno);
		return 1;
	}
	if(!lck_wine.success()) {
		printf("Can't open Wine map\n");
		return 1;
	}
    WineSHM* shm_posix = (WineSHM*) lck_posix.ptr();
    FTHeap* shm_wine = (FTHeap*) lck_wine.ptr();
    FTData* data = &shm_wine->data;
    create_registry_key();
	while (1) {
		lck_posix.lock();
		if (shm_posix->stop) {
			lck_posix.unlock();
			break;
		}
		lck_wine.lock();
        data->Yaw = shm_posix->data[Yaw];
        data->Pitch = shm_posix->data[Pitch];
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
		lck_wine.unlock();
		lck_posix.unlock();
        (void) Sleep(4);
	}
}
