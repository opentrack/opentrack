#include "ftnoir_tracker_freepie-udp.h"
#include "api/plugin-api.hpp"
#include "compat/util.hpp"

#include <cinttypes>
#include <algorithm>
#include <cmath>

tracker_freepie::tracker_freepie() : pose { 0,0,0, 0,0,0 }
{
}

tracker_freepie::~tracker_freepie()
{
    requestInterruption();
    wait();
}

void tracker_freepie::run() {
#pragma pack(push, 1)
    struct {
        uint8_t pad1;
        uint8_t flags;
        float fl[12];
    } data;
#pragma pack(pop)

    enum F
    {
        flag_Raw = 1 << 0,
        flag_Orient = 1 << 1,
        Mask = flag_Raw | flag_Orient
    };

    sock.bind(QHostAddress::Any, (unsigned short) s.port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    while (!isInterruptionRequested())
    {
        int order[] =
        {
            clamp(s.idx_x, 0, 2),
            clamp(s.idx_y, 0, 2),
            clamp(s.idx_z, 0, 2)
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
            static constexpr int add_cbx[] =
            {
                0,
                90,
                -90,
                180,
                -180,
            };

            int add_indices[] = { s.add_yaw, s.add_pitch, s.add_roll };

            QMutexLocker foo(&mtx);

            constexpr double r2d = 180 / M_PI;

            for (int i = 0; i < 3; i++)
            {
                const int axis = order[i];
                const int add_idx = add_indices[i];
                int add = 0;
                if (add_idx >= 0 && add_idx < (int)std::size(add_cbx))
                    add = add_cbx[add_idx];
                pose[Yaw + i] = r2d * orient[axis] + add;
            }
        }
        usleep(4000);
    }
}

module_status tracker_freepie::start_tracker(QFrame*)
{
    start();
    sock.moveToThread(this);

    return status_ok();
}

void tracker_freepie::data(double *data)
{
    QMutexLocker foo(&mtx);

    data[Yaw] = pose[Yaw];
    data[Pitch] = pose[Pitch];
    data[Roll] = pose[Roll];
}

OPENTRACK_DECLARE_TRACKER(tracker_freepie, dialog_freepie, meta_freepie)
