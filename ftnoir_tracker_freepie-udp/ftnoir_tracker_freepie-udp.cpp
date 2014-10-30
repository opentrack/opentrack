#include "ftnoir_tracker_freepie-udp.h"
#include "opentrack/plugin-api.hpp"

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

template<typename t>
static const t bound(t datum, t least, t max)
{
    if (datum < least)
        return least;
    if (datum > max)
        return max;
    return datum;
}

void TrackerImpl::run() {
#pragma pack(push, 1)
    struct {
        uint8_t pad1;
        uint8_t flags;
        float fl[12];
    } data;
#pragma pack(pop)
    enum F {
        flag_Raw = 1 << 0,
        flag_Orient = 1 << 1,
        Mask = flag_Raw | flag_Orient
    };

    while (!should_quit) {
        int order[] = {
            bound<int>(s.idx_x, 0, 2),
            bound<int>(s.idx_y, 0, 2),
            bound<int>(s.idx_z, 0, 2)
        };
        float orient[3];
        bool filled = false;

        while (sock.hasPendingDatagrams())
        {
            using t = decltype(data);
            t tmp {0,0, {0,0,0, 0,0,0}};
            (void) sock.readDatagram(reinterpret_cast<char*>(&tmp), sizeof(data));

            int flags = tmp.flags & F::Mask;

            switch (flags)
            {
            case flag_Raw:
            continue;
            case flag_Raw | flag_Orient:
                for (int i = 0; i < 3; i++)
                    orient[i] = tmp.fl[i+9];
            break;
            case flag_Orient:
                for (int i = 0; i < 3; i++)
                    orient[i] = tmp.fl[i];
            break;
            }

            filled = true;
            data = tmp;
        }

        if (filled)
        {
            QMutexLocker foo(&mtx);
            static constexpr double d2r = 57.295781;
            for (int i = 0; i < 3; i++)
            {
                pose[Yaw + i] = d2r * orient[order[i]];
            }
        }
        usleep(4000);
    }
}

void TrackerImpl::start_tracker(QFrame*)
{
    (void) sock.bind(QHostAddress::Any, (int) s.port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    start();
}

void TrackerImpl::data(double *data)
{
    QMutexLocker foo(&mtx);

    data[Yaw] = pose[Yaw];
    data[Pitch] = pose[Pitch];
    data[Roll] = pose[Roll];
}

extern "C" OPENTRACK_EXPORT ITracker* GetConstructor()
{
    return new TrackerImpl;
}
