#include "ftnoir_tracker_s2bot.h"
#include "api/plugin-api.hpp"

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
    requestInterruption();
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

void tracker_s2bot::run() {
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

	if (s.freq == 0) s.freq = 10;
	timer.setInterval(1000.0/s.freq);
	timer.setSingleShot(false);	
	connect(&timer, &QTimer::timeout, [this]() {
		auto reply = m_nam->get(QNetworkRequest(QUrl("http://localhost:17317/poll")));
		connect(reply, &QNetworkReply::finished, [this, reply]() {
			if (reply->error() == QNetworkReply::NoError) {
				qDebug() << "Request submitted OK";
			}
			else {
				qWarning() << "Request bounced:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute) << reply->errorString();
				return;
			}
			QByteArray ba = reply->readAll();
			QStringList slist = QString(ba).split(QRegExp("[\r\n]"), QString::SkipEmptyParts);
			int order[] = {
				bound<int>(s.idx_x, 0, 3),
				bound<int>(s.idx_y, 0, 3),
				bound<int>(s.idx_z, 0, 3)
			};
			double orient[4] = { 0, 0, 0, 0 };
			static const int add_cbx[] = {
				0,
				90,
				-90,
				180,
				-180,
			};
			int indices[] = { s.add_yaw, s.add_pitch, s.add_roll };
			for (auto line : slist) {
				QStringList keyval = line.split(" ");
				if (keyval.count() < 2) continue;
				if (keyval[0].startsWith("accelerometerZ")) orient[0] = keyval[1].toInt();
				else if (keyval[0].startsWith("accelerometerY")) orient[1] = keyval[1].toInt();
				else if (keyval[0].startsWith("accelerometerX")) orient[2] = keyval[1].toInt();
				else if (keyval[0].startsWith("bearing")) orient[3] = keyval[1].toInt();
			}
			QMutexLocker foo(&mtx);
			static constexpr double r2d = 180 / M_PI;
			for (int i = 0; i < 3; i++)
			{
				int val = 0;
				int idx = indices[order[i]];
				if (idx >= 0 && idx < (int)(sizeof(add_cbx) / sizeof(*add_cbx)))
					val = add_cbx[idx];
				pose[Yaw + i] = orient[order[i]] + val; // * r2d if it was radians
			}
			reply->close();
			reply->deleteLater();
		});
	});
	timer.start();
	exec();
}

void tracker_s2bot::start_tracker(QFrame*)
{
    start();
    timer.moveToThread(this);
}

void tracker_s2bot::data(double *data)
{
    QMutexLocker foo(&mtx);

    data[Yaw] = pose[Yaw];
    data[Pitch] = pose[Pitch];
    data[Roll] = pose[Roll];
}

OPENTRACK_DECLARE_TRACKER(tracker_s2bot, dialog_s2bot, meta_s2bot)
