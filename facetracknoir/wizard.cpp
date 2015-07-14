#include "wizard.h"

Wizard::Wizard() : QWizard(nullptr)
{
    ui.setupUi(this);
    connect(this, SIGNAL(accepted()), this, SLOT(set_data()));
}

static constexpr double tz[][2] = {
    { 16.5327205657959, 13.0232553482056 },
    { 55.4535026550293, 100 },
    { 56.8312301635742, 100 },
    { -1, -1 },
};

static constexpr double yaw[][2] = {
    { 10.7462686567164, 20.9302325581395 },
    { 41.9517784118652, 180 },
    { -1, -1 },
};

static constexpr double pitch[][2] = {
    { 10.1262916188289, 27.6279069767442 },
    { 32.4454649827784, 180 },
    { -1, -1 },
};

static constexpr double roll[][2] = {
    { 12.3995409011841, 25.9534893035889 },
    { 54.3513221740723, 180 },
    { -1, -1 },
};

static void set_mapping(Mapping& m, double* spline[2])
{
    m.opts.altp = false;
    m.curve.removeAllPoints();
    for (int i = 0; spline[i][0] >= 0; i++)
        m.curve.addPoint(QPointF(spline[i][0], spline[i][1]));
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

    pt.threshold = 31;

    qDebug() << "wizard done" << "model" << m << "camera-mode" << camera_mode;
}
