#pragma once
#include "ui_kinect_face_settings.h"
#include "compat/macros.hpp"
#include "api/plugin-api.hpp"


class KinectFaceSettings : public ITrackerDialog
{
    Q_OBJECT

    Ui::KinectFaceUi ui;
public:
    KinectFaceSettings();
    void register_tracker(ITracker *) override {}
    void unregister_tracker() override {}
private slots:
    void doOK();
    void doCancel();
};

class KinectFaceMetadata : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("Kinect Face 0.1"); }
    QIcon icon() override { return QIcon(":/images/kinect.png"); }
};

