#include "ftnoir_tracker_freepie-udp.h"
#include "api/plugin-api.hpp"

#include <cinttypes>
#include <algorithm>
#include <cmath>


tracker_freepie::tracker_freepie() : pose { 0,0,0, 0,0,0 }, should_quit(false)
{
}

tracker_freepie::~tracker_freepie()
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

void tracker_freepie::run() {
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

    sock.bind(QHostAddress::Any, (unsigned short) s.port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    while (!should_quit) {
        int order[] = {
            bound<int>(s.idx_x, 0, 2),
            bound<int>(s.idx_y, 0, 2),
            bound<int>(s.idx_z, 0, 2)
        };
        double orient[3] = {0, 0, 0};
        bool filled = false;

        while (sock.hasPendingDatagrams())
        {
            using t = decltype(data);
            t tmp {0,0, {0,0,0, 0,0,0, 0,0,0, 0,0,0}};
            (void) sock.readDatagram(reinterpret_cast<char*>(&tmp), sizeof(data));

            int flags = tmp.flags & F::Mask;

            switch (flags)
            {
            //default:
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
            static const int add_cbx[] = {
                0,
                90,
                -90,
                180,
                -180,
            };
            int indices[] = { s.add_yaw, s.add_pitch, s.add_roll };
            QMutexLocker foo(&mtx);
            static constexpr double r2d = 180 / M_PI;
            for (int i = 0; i < 3; i++)
            {
                int val = 0;
                int idx = indices[order[i]];
                if (idx >= 0 && idx < (int)(sizeof(add_cbx) / sizeof(*add_cbx)))
                    val = add_cbx[idx];
                pose[Yaw + i] = r2d * orient[order[i]] + val;
            }
        }
        usleep(4000);
    }
}

void tracker_freepie::start_tracker(QFrame*)
{
    start();
    sock.moveToThread(this);
}

void tracker_freepie::data(double *data)
{
    QMutexLocker foo(&mtx);

    data[Yaw] = pose[Yaw];
    data[Pitch] = pose[Pitch];
    data[Roll] = pose[Roll];
}

OPENTRACK_DECLARE_TRACKER(tracker_freepie, dialog_freepie, meta_freepie)
