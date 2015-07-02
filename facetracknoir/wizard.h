#pragma once

#include "opentrack/options.hpp"
#include "opentrack/main-settings.hpp"
#include "opentrack/mappings.hpp"
#include "ui_trackhat-wizard.h"
#include "ftnoir_tracker_pt/ftnoir_tracker_pt_settings.h"
#include <QWizard>

class Wizard : public QWizard
{
    Q_OBJECT
    Ui_Form ui;
    settings_pt pt;
    main_settings s;
public:
    Wizard(QWidget* parent);
};
