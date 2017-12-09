#pragma once
#include "api/plugin-api.hpp"
#include "api/plugin-support.hpp"
#include <QObject>
#include <QFrame>
#include <QCoreApplication>

#include "options/options.hpp"
using namespace options;

struct fusion_settings final : opts
{
    value<QVariant> rot_tracker_name, pos_tracker_name;

    fusion_settings();
};

class fusion_tracker : public QObject, public ITracker
{
    Q_OBJECT

    double rot_tracker_data[6] {}, pos_tracker_data[6] {};

    std::unique_ptr<QFrame> other_frame;
    std::shared_ptr<dylib> rot_dylib, pos_dylib;
    std::shared_ptr<ITracker> rot_tracker, pos_tracker;

public:
    fusion_tracker();
    ~fusion_tracker() override;
    module_status start_tracker(QFrame*) override;
    void data(double* data) override;

    static const QString& caption();
};

#include "ui_fusion.h"

class fusion_dialog : public ITrackerDialog
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
    QString name() { return otr_tr("Fusion"); }
    QIcon icon() { return QIcon(":/images/fusion-tracker-logo.png"); }
};

