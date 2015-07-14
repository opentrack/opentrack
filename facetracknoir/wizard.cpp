#include "wizard.h"

Wizard::Wizard() : QWizard(nullptr)
{
    ui.setupUi(this);
    connect(this, SIGNAL(accepted()), this, SLOT(set_data()));
}

void Wizard::set_data()
{
    Model m;

    if (ui.clip_model->isChecked())
        m = ClipRight;
    else if (ui.clip_model_left->isChecked())
        m = ClipLeft;
    else // ui.cap_model
        m = Cap;

    auto camera_mode = static_cast<CameraMode>(ui.resolution_select->currentIndex());

    settings_pt pt;
    main_settings s;

    qDebug() << "wizard done" << "model" << m << "camera-mode" << camera_mode;
}
