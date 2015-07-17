#include "wizard.h"
#include "opentrack/state.hpp"
#include "ftnoir_tracker_pt/ftnoir_tracker_pt_settings.h"
#include "ftnoir_filter_accela/ftnoir_filter_accela.h"

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

static void set_mapping(Mapping& m, const double spline[][2])
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
    State state;

    set_mapping(state.pose(TZ), tz);
    set_mapping(state.pose(Yaw), yaw);
    set_mapping(state.pose(Pitch), pitch);
    set_mapping(state.pose(Roll), roll);

    pt.threshold = 31;
    pt.min_point_size = 2;
    pt.max_point_size = 50;

    switch (m)
    {
    default:
    case Cap: pt.t_MH_x = 0; pt.t_MH_y = 0; pt.t_MH_z = 0; break;
    case ClipRight: pt.t_MH_x = ClipRightX; pt.t_MH_y = 0; pt.t_MH_z = 0; break;
    case ClipLeft: pt.t_MH_x = ClipLeftX; pt.t_MH_y = 0; pt.t_MH_z = 0; break;
    }

    pt.camera_mode = camera_mode;
    pt.fov = ui.camera_fov->currentIndex();

    settings_accela acc;
    acc.ewma = 49;
    acc.rot_threshold = 29;
    acc.rot_deadzone = 29;
    acc.trans_deadzone = 33;
    acc.trans_threshold = 19;

    acc.b->save();
    pt.b->save();

    qDebug() << "wizard done" << "model" << m << "camera-mode" << camera_mode;
}
