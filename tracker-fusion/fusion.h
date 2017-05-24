#pragma once
#include "api/plugin-api.hpp"
#include "api/plugin-support.hpp"
#include <QFrame>
#include <QCoreApplication>

#include "options/options.hpp"
using namespace options;

struct fusion_settings final : opts
{
    value<QString> rot_tracker_name;
    value<QString> pos_tracker_name;

    fusion_settings();
};

struct has_modules
{
    Modules modules;
    has_modules();
};

struct fusion_tracker : public ITracker, has_modules
{
    fusion_tracker();
    ~fusion_tracker() override;
    void start_tracker(QFrame *) override;
    void data(double *data) override;

    double rot_tracker_data[6], pos_tracker_data[6];

    std::unique_ptr<QFrame> other_frame;
    std::shared_ptr<dylib> rot_dylib, pos_dylib;
    std::shared_ptr<ITracker> rot_tracker, pos_tracker;
};

#include "ui_fusion.h"

class fusion_dialog : public ITrackerDialog, has_modules
{
    Q_OBJECT

    fusion_settings s;
    Ui::fusion_ui ui;
public:
    fusion_dialog();
private slots:
    void doOK();
    void doCancel();
};

class fusion_metadata : public Metadata
{
public:
    QString name() { return QString(QCoreApplication::translate("fusion_metadata", "Fusion")); }
    QIcon icon() { return QIcon(":/images/fusion-tracker-logo.png"); }
};

