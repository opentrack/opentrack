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
	ShmWine lck_wine("FT_SharedMem", "FT_Mutext", sizeof(TFreeTrackData));
	if(lck_posix.mem == (void*)-1) {
		printf("Can't open posix map: %d\n", errno);
		return 1;
	}
	if(lck_wine.mem == NULL) {
		printf("Can't open Wine map\n");
		return 1;
	}
	WineSHM* shm_posix = (WineSHM*) lck_posix.mem;
	TFreeTrackData* shm_wine = (TFreeTrackData*) lck_wine.mem;
	while (!shm_posix->stop) {
		(void) Sleep(10);
		lck_posix.lock();
		if (shm_posix->stop) {
			lck_posix.unlock();
			break;
		}
		lck_wine.lock();
		shm_wine->Yaw = shm_posix->rx;
		shm_wine->Pitch = shm_posix->ry;
		shm_wine->Roll = shm_posix->rz;
		shm_wine->X = shm_posix->tx;
		shm_wine->Y = shm_posix->ty;
		shm_wine->Z = shm_posix->tz;
		shm_wine->DataID = 1;
		shm_wine->CamWidth = 2;
		shm_wine->CamHeight = 3;
		lck_wine.unlock();
		lck_posix.unlock();
	}
}
