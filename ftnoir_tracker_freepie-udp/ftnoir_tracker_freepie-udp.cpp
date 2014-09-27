#include "ftnoir_tracker_freepie-udp.h"
#include "facetracknoir/plugin-support.h"

#include <cinttypes>
#include <algorithm>

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

    struct check {
        union {
            std::uint16_t half;
            unsigned char bytes[2];
        };
        bool convertp;
        check() : bytes { 0, 255 }, convertp(half > 255) {}
    } crapola;

    while (1) {
        if (should_quit)
            break;

        float* orient = nullptr;

        while (sock.hasPendingDatagrams())
        {
            using t = decltype(data);
            t tmp {0,0, {0,0,0, 0,0,0}};
            (void) sock.readDatagram(reinterpret_cast<char*>(&tmp), sizeof(data));
            int flags = data.flags & F::Mask;

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
            data = tmp;
        }
        if (orient)
        {
            if (crapola.convertp)
            {
                unsigned char* alias = reinterpret_cast<unsigned char*>(orient);
                constexpr int sz = sizeof(float[6]);
                std::reverse(alias, alias + sz);
            }

            QMutexLocker foo(&mtx);
            for (int i = 0; i < 3; i++)
                pose[Yaw + i] = orient[i];

            orient = nullptr;
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

extern "C" OPENTRACK_EXPORT ITracker* GetConstructor()
{
    return new TrackerImpl;
}
