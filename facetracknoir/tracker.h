#ifndef __TRACKER_H__
#define __TRACKER_H__

#include <QThread>
#include <QMessageBox>
#include <QLineEdit>
#include <QPoint>
#include <QWaitCondition>
#include <QList>
#include <QPainterPath>
#include <QDebug>
#include <QMutex>
#include "global-settings.h"
#include <ftnoir_tracker_base/ftnoir_tracker_types.h>
#include <vector>

#include <qfunctionconfigurator/functionconfig.h>
#include "tracker_types.h"
#include "facetracknoir/main-settings.hpp"
#include "facetracknoir/options.h"
using namespace options;

class FaceTrackNoIR;				// pre-define parent-class to avoid circular includes

class THeadPoseDOF {
private:
    THeadPoseDOF(const THeadPoseDOF &) = delete;
    THeadPoseDOF& operator=(const THeadPoseDOF&) = delete;
public:
    THeadPoseDOF(QString primary,
                 QString secondary,
                 int maxInput1,
                 int maxOutput1,
                 int maxInput2,
                 int maxOutput2,
                 axis_opts* opts) :
        headPos(0),
        curve(primary, maxInput1, maxOutput1),
        curveAlt(secondary, maxInput2, maxOutput2),
        opts(*opts)
    {
        QSettings settings("opentrack");
        QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
        QSettings iniFile( currentFile, QSettings::IniFormat );
        curve.loadSettings(iniFile);
        curveAlt.loadSettings(iniFile);
    }
    volatile double headPos;
    FunctionConfig curve;
	FunctionConfig curveAlt;
    axis_opts& opts;
};

class Tracker : protected QThread {
	Q_OBJECT

private:
    FaceTrackNoIR *mainApp;
    QMutex mtx;
    main_settings& s;
    volatile bool should_quit;
protected:
	void run();

public:
    Tracker( FaceTrackNoIR *parent, main_settings& s);
    ~Tracker();

    void getHeadPose(double *data);
    void getOutputHeadPose(double *data);
    volatile bool do_center;
    volatile bool enabled;
    
    T6DOF output_camera;

    void start() { QThread::start(); }
};

class HeadPoseData {
public:
    THeadPoseDOF* axes[6];
    HeadPoseData(std::vector<axis_opts*> opts)
    {
        axes[TX] = new THeadPoseDOF("tx","tx_alt", 100, 100, 100, 100, opts[TX]);
        axes[TY] = new THeadPoseDOF("ty","ty_alt", 100, 100, 100, 100, opts[TY]);
        axes[TZ] = new THeadPoseDOF("tz","tz_alt", 100, 100, 100, 100, opts[TZ]);
        axes[Yaw] = new THeadPoseDOF("rx", "rx_alt", 180, 180, 180, 180, opts[Yaw]);
        axes[Pitch] = new THeadPoseDOF("ry", "ry_alt", 90, 90, 90, 90, opts[Pitch]);
        axes[Roll] = new THeadPoseDOF("rz", "rz_alt", 180, 180, 180, 180, opts[Roll]);
    }
    ~HeadPoseData()
    {
        for (int i = 0; i < 6; i++)
        {
            delete axes[i];
        }
    }
};

#endif
