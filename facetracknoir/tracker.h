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
#include "plugin-support.h"
#include <vector>

#include <qfunctionconfigurator/functionconfig.h>
#include "tracker_types.h"
#include "facetracknoir/main-settings.hpp"
#include "facetracknoir/options.h"
#include "facetracknoir/timer.hpp"
using namespace options;

class THeadPoseDOF {
public:
    THeadPoseDOF(QString primary,
                 QString secondary,
                 int maxInput1,
                 int maxOutput1,
                 int maxInput2,
                 int maxOutput2,
                 axis_opts* opts) :
        headPos(0),
        curve(maxInput1, maxOutput1),
        curveAlt(maxInput2, maxOutput2),
        opts(*opts),
        name1(primary),
        name2(secondary)
    {
        QSettings settings("opentrack");
        QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
        QSettings iniFile( currentFile, QSettings::IniFormat );
        curve.loadSettings(iniFile, primary);
        curveAlt.loadSettings(iniFile, secondary);
    }
    volatile double headPos;
    FunctionConfig curve;
	FunctionConfig curveAlt;
    axis_opts& opts;
    QString name1, name2;
};

class FaceTrackNoIR;

class Tracker : protected QThread {
	Q_OBJECT

private:
    FaceTrackNoIR *mainApp;
    QMutex mtx;
    main_settings& s;
    volatile bool should_quit;
    Timer t;
protected:
	void run();

public:
    Tracker( FaceTrackNoIR *parent, main_settings& s);
    ~Tracker();

    void getHeadPose(double *data);
    void getOutputHeadPose(double *data);
    volatile bool do_center;
    volatile bool enabled;
    
    T6DOF output_camera, raw_6dof;

    void start() { QThread::start(); }
};

class HeadPoseData {
public:
    THeadPoseDOF axes[6];
    HeadPoseData(std::vector<axis_opts*> opts) :
        axes {
            THeadPoseDOF("tx","tx_alt", 100, 100, 100, 100, opts[TX]),
            THeadPoseDOF("ty","ty_alt", 100, 100, 100, 100, opts[TY]),
            THeadPoseDOF("tz","tz_alt", 100, 100, 100, 100, opts[TZ]),
            THeadPoseDOF("rx", "rx_alt", 180, 180, 180, 180, opts[Yaw]),
            THeadPoseDOF("ry", "ry_alt", 90, 90, 90, 90, opts[Pitch]),
            THeadPoseDOF("rz", "rz_alt", 180, 180, 180, 180, opts[Roll])
        }
    {}
};

#endif
