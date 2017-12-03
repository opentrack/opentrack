#include "ftnoir_tracker_s2bot.h"
#include "api/plugin-api.hpp"
#include "compat/util.hpp"

#include <cinttypes>
#include <algorithm>
#include <cmath>
#include <QNetworkRequest>
#include <QNetworkReply>

tracker_s2bot::tracker_s2bot() : pose { 0,0,0, 0,0,0 }, m_nam (std::make_unique<QNetworkAccessManager>())
{
}

tracker_s2bot::~tracker_s2bot()
{
    QThread::exit(0); // stop event loop
    wait();
}

void tracker_s2bot::run() {
    if (s.freq == 0) s.freq = 10;
    timer.setInterval(1000.0/s.freq);
    timer.setSingleShot(false);
    connect(&timer, &QTimer::timeout, [this]() {
        auto reply = m_nam->get(QNetworkRequest(QUrl("http://localhost:17317/poll")));
        connect(reply, &QNetworkReply::finished, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                //qDebug() << "Request submitted OK";
            }
            else {
                qWarning() << "Request bounced:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << reply->errorString();
                return;
            }

            const QStringList slist = QString::fromLatin1(reply->readAll()).split(QRegExp("[\r\n]+"), QString::SkipEmptyParts);
            reply->close();
            reply->deleteLater();

            int order[] =
            {
                clamp(s.idx_x, 0, 3),
                clamp(s.idx_y, 0, 3),
                clamp(s.idx_z, 0, 3),
            };

            static constexpr int add_cbx[] =
            {
                0,
                90,
                -90,
                180,
                -180,
            };

            int add_indices[] = { s.add_yaw, s.add_pitch, s.add_roll, };
            double orient[4] {};

            for (auto line : slist)
            {
                QStringList keyval = line.split(' ');
                if (keyval.count() < 2) continue;
                if (keyval[0].startsWith("accelerometerZ")) orient[0] = keyval[1].toDouble();
                else if (keyval[0].startsWith("accelerometerY")) orient[1] = keyval[1].toDouble();
                else if (keyval[0].startsWith("accelerometerX")) orient[2] = keyval[1].toDouble();
                else if (keyval[0].startsWith("bearing")) orient[3] = keyval[1].toDouble();
            }

            QMutexLocker foo(&mtx);

            for (int i = 0; i < 3; i++)
            {
                const int axis = order[i];
                const int add_idx = add_indices[i];
                int add = 0;
                if (add_idx >= 0 && add_idx < (int)std::size(add_cbx))
                    add = add_cbx[add_idx];
                pose[Yaw + i] = orient[axis] + add; // * r2d if it was radians
            }
        });
    });

    timer.start();
    exec();
    timer.stop();
}

module_status tracker_s2bot::start_tracker(QFrame*)
{
    start();
    timer.moveToThread(this);
    m_nam->moveToThread(this);

    return status_ok();
}

void tracker_s2bot::data(double *data)
{
    QMutexLocker foo(&mtx);

    data[Yaw] = pose[Yaw];
    data[Pitch] = pose[Pitch];
    data[Roll] = pose[Roll];
}

OPENTRACK_DECLARE_TRACKER(tracker_s2bot, dialog_s2bot, meta_s2bot)
