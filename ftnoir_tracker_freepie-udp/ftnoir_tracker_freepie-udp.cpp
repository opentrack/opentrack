#include "ftnoir_tracker_freepie-udp.h"
#include "facetracknoir/plugin-support.h"

#include <stddef.h>
#include <cinttypes>

TrackerImpl::TrackerImpl() : pose { 0,0,0, 0,0,0 }, should_quit(false)
{
}

TrackerImpl::~TrackerImpl()
{
    should_quit = true;
    wait();
}

void TrackerImpl::run() {
    struct {
        uint8_t pad;
        uint8_t flags;
        union {
            float rot[6];
            struct {
                float pad[9];
                float rot[6];
            } raw_rot;
        };
    } data;

    enum F {
        flag_Raw = 1 << 0,
        flag_Orient = 1 << 1,
        Mask = flag_Raw | flag_Orient
    };

    while (1) {
        if (should_quit)
            break;
        {
            float* orient = nullptr;

            while (sock.hasPendingDatagrams())
            {
                data = decltype(data){0,0, 0,0,0};
                int sz = sock.readDatagram(reinterpret_cast<char*>(&data), sizeof(data));

                int flags = data.flags & F::Mask;
                
                constexpr int minsz = offsetof(decltype(data), raw_rot) + sizeof(decltype(data)::raw_rot);
                const bool flags_came_out_wrong = minsz > sz;
                
                if (flags_came_out_wrong)
                    flags &= ~F::flag_Raw;
                
                switch (flags)
                {
                case flag_Raw:
                    continue;
                case flag_Raw | flag_Orient:
                    orient = data.raw_rot.rot;
                    break;
                case flag_Orient:
                    orient = data.rot;
                    break;
                }
            }
            if (orient)
            {
                QMutexLocker foo(&mtx);
                for (int i = 0; i < 3; i++)
                    pose[Yaw + i] = orient[i];
            }
        }
        usleep(4000);
    }
}

void TrackerImpl::StartTracker(QFrame*)
{
    (void) sock.bind(QHostAddress::Any, (int) s.port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
  start();
}

void TrackerImpl::GetHeadPoseData(double *data)
{
    QMutexLocker foo(&mtx);
#if 0
    if (s.enable_x)
        data[TX] = pose[TX];
    if (s.enable_y)
        data[TY] = pose[TY];
    if (s.enable_z)
        data[TZ] = pose[TZ];
#endif
    data[Yaw] = pose[Yaw];
    data[Pitch] = pose[Pitch];
    data[Roll] = pose[Roll];
}

extern "C" OPENTRACK_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
{
    return new TrackerImpl;
}
