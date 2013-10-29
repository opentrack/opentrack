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

#include <qfunctionconfigurator/functionconfig.h>
#include "tracker_types.h"

class FaceTrackNoIR;				// pre-define parent-class to avoid circular includes

class THeadPoseDOF {
private:
    THeadPoseDOF(const THeadPoseDOF &) {}
public:
    THeadPoseDOF() :
        headPos(0),
        invert(0),
        altp(false),
        zero(0)
    {
    }

    THeadPoseDOF(QString primary,
                 QString secondary,
                 int maxInput1,
                 int maxOutput1,
                 int maxInput2,
                 int maxOutput2) :
        headPos(0),
        invert(1),
        curve(primary, maxInput1, maxOutput1),
        curveAlt(secondary, maxInput2, maxOutput2),
        zero(0)
    {
        QSettings settings("opentrack");
        QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
        QSettings iniFile( currentFile, QSettings::IniFormat );
        curve.loadSettings(iniFile);
        curveAlt.loadSettings(iniFile);
        
        iniFile.beginGroup("Tracking");
        altp = iniFile.value(secondary).toBool();
        iniFile.endGroup();
    }
    volatile double headPos;
    volatile float invert;
    FunctionConfig curve;
	FunctionConfig curveAlt;
    volatile bool altp;
    volatile double zero;
};

class Tracker : public QThread {
	Q_OBJECT

private:
    FaceTrackNoIR *mainApp;
    QMutex mtx;

protected:
	void run();

public:
	Tracker( FaceTrackNoIR *parent );
    ~Tracker();

    void setInvertAxis(Axis axis, bool invert);

    void getHeadPose(double *data);
    void getOutputHeadPose(double *data);

    volatile bool should_quit;
    volatile bool do_center;
    volatile bool enabled;
    volatile bool compensate;
    
    T6DOF output_camera;
};

class HeadPoseData {
public:
    THeadPoseDOF* axes[6];
    HeadPoseData()
    {
        axes[TX] = new THeadPoseDOF("tx","tx_alt", 100, 100, 100, 100);
        axes[TY] = new THeadPoseDOF("ty","ty_alt", 100, 100, 100, 100);
        axes[TZ] = new THeadPoseDOF("tz","tz_alt", 100, 100, 100, 100);
        axes[Yaw] = new THeadPoseDOF("rx", "rx_alt", 180, 180, 180, 180);
        axes[Pitch] = new THeadPoseDOF("ry", "ry_alt", 90, 90, 90, 90);
        axes[Roll] = new THeadPoseDOF("rz", "rz_alt", 180, 180, 180, 180);
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
