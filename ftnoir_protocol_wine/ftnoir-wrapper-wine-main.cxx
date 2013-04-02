#include <errno.h>
#include <stdio.h>
#include "ftnoir_protocol_wine/fttypes.h"
class ShmPosix {
public:
    ShmPosix(const char *shmName, const char *mutexName, int mapSize);
    ~ShmPosix();
    void lock();
    void unlock();
    void* mem;
private:
    int fd, size;
};

class ShmWine {
public:
    ShmWine(const char *shmName, const char *mutexName, int mapSize);
    ~ShmWine();
    void lock();
    void unlock();
    void* mem;
private:
    void *hMutex, *hMapFile;
};
#include <windows.h>

int main(void)
{
	ShmPosix lck_posix(WINE_SHM_NAME, WINE_MTX_NAME, sizeof(WineSHM));
    ShmWine lck_wine("FT_SharedMem", "FT_Mutext", sizeof(FTMemMap));
	if(lck_posix.mem == (void*)-1) {
		printf("Can't open posix map: %d\n", errno);
		return 1;
	}
	if(lck_wine.mem == NULL) {
		printf("Can't open Wine map\n");
		return 1;
	}
	WineSHM* shm_posix = (WineSHM*) lck_posix.mem;
    FTMemMap* shm_wine = (FTMemMap*) lck_wine.mem;
    TFreeTrackData* data = &shm_wine->data;
	while (!shm_posix->stop) {
		(void) Sleep(10);
		lck_posix.lock();
		if (shm_posix->stop) {
			lck_posix.unlock();
			break;
		}
		lck_wine.lock();
        data->Yaw = shm_posix->rx;
        data->Pitch = shm_posix->ry;
        data->Roll = shm_posix->rz;
        data->X = shm_posix->tx;
        data->Y = shm_posix->ty;
        data->Z = shm_posix->tz;
        data->DataID = 1;
        data->CamWidth = 250;
        data->CamHeight = 100;
		lck_wine.unlock();
		lck_posix.unlock();
	}
}
