#define OPENTRACK_COMPAT_BUNDLED
#define PortableLockedShm ShmPosix
#undef _WIN32
#include "ftnoir_protocol_ft/fttypes.h"
#include "wine-shm.h"
#include "compat/compat.h"
#include "compat/compat.cpp"

ptr<BasePortableLockedShm> make_shm_posix()
{
    return std::make_shared<ShmPosix>(FREETRACK_HEAP, FREETRACK_MUTEX, sizeof(FTHeap));
}
