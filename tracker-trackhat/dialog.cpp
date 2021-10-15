#include "dialog.hpp"

using namespace options;

trackhat_dialog::trackhat_dialog()
{
    ui.setupUi(this);
    poll_tracker_info();
    poll_timer.setInterval(250);

    struct {
        const char* name;
        model_type t;
    } model_types[] = {
        { "Cap", model_cap },
        { "Clip (left)", model_clip_left },
        { "Clip (right)", model_clip_right },
        { "Mini Clip (left)", model_mini_clip_left },
        { "Mini Clip (right)", model_mini_clip_right },
    };

    ui.model_type->clear();

    for (auto x : model_types)
        ui.model_type->addItem(QIcon{}, tr(x.name), (QVariant)(int)x.t);

    tie_setting(t.min_pt_size, ui.min_point_size);
    tie_setting(t.max_pt_size, ui.max_point_size);
    tie_setting(t.exposure, ui.exposure_slider);
    tie_setting(t.model, ui.model_type);
    tie_setting(t.threshold, ui.threshold_slider);
    tie_setting(t.enable_point_filter, ui.enable_point_filter);
    tie_setting(t.point_filter_coefficient, ui.point_filter_slider);
    connect(&t.exposure, value_::value_changed<slider_value>(), ui.exposure_label,
            [this] { ui.exposure_label->setValue((int)*t.exposure); }, Qt::QueuedConnection);
    connect(&t.threshold, value_::value_changed<slider_value>(), ui.threshold_label,
            [this] { ui.threshold_label->setValue((int)*t.threshold); }, Qt::QueuedConnection);
    connect(&t.point_filter_coefficient, value_::value_changed<slider_value>(), ui.point_filter_label,
            [this] { ui.point_filter_label->setValue((int)*t.point_filter_coefficient); }, Qt::QueuedConnection);
    connect(&poll_timer, &QTimer::timeout, this, &trackhat_dialog::poll_tracker_info);

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &trackhat_dialog::doOK);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &trackhat_dialog::doCancel);
}

void trackhat_dialog::register_tracker(ITracker* tracker)
{
    tracker = static_cast<Tracker_PT*>(tracker);
    poll_timer.start();
}

void trackhat_dialog::unregister_tracker()
{
    tracker = nullptr;
    poll_timer.stop();
}

void trackhat_dialog::doOK()
{
    s.b->save();
    t.b->save();
    close();
}

void trackhat_dialog::doCancel()
{
    close();
}

void trackhat_dialog::poll_tracker_info()
{
    if (!tracker)
    {
        poll_timer.stop();
        ui.status_label->setText(tr("Tracking stopped."));
        return;
    }

    ui.status_label->setText(tr("Tracking. %1 points detected.").arg(tracker->get_n_points()));
}

trackhat_dialog::~trackhat_dialog() = default;
