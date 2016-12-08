#include "tobii-eyex.hpp"

tobii_eyex_dialog::tobii_eyex_dialog()
{
    ui.setupUi(this);

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &tobii_eyex_dialog::do_ok);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &tobii_eyex_dialog::do_cancel);

    ui.tracking_mode->addItem("Relative", tobii_relative);
    ui.tracking_mode->addItem("Absolute", tobii_absolute);

    tie_setting(s.mode, ui.tracking_mode);

    ui.relative_mode_gain->setConfig(&rs.acc_mode_spline);
    ui.relative_mode_gain->set_preview_only(true);

    tie_setting(rs.dz_len, ui.deadzone);
    tie_setting(rs.expt_slope, ui.exponent);
    tie_setting(rs.expt_len, ui.exponent_len);
    tie_setting(rs.expt_norm, ui.exponent_norm);

    tie_setting(rs.log_slope, ui.log_base);
    tie_setting(rs.log_len, ui.log_len);
    tie_setting(rs.log_norm, ui.log_norm);

    connect(rs.b.get(), &bundle_::changed, this, [this]() { rs.make_spline(); }, Qt::QueuedConnection);

    // todo add specialization for label with traits
#if 0
    tie_setting(rs.dz_len, ui.deadzone_label);
    tie_setting(rs.expt_slope, ui.exponent_label);
    tie_setting(rs.expt_len, ui.exponent_len_label);
    tie_setting(rs.expt_norm, ui.exponent_norm_label);
    tie_setting(rs.log_slope, ui.log_base_label);
    tie_setting(rs.log_len, ui.log_len_label);
    tie_setting(rs.log_norm, ui.log_norm_label);
#endif
}

void tobii_eyex_dialog::do_ok()
{
    s.b->save();
    rs.b->save();
    close();
}

void tobii_eyex_dialog::do_cancel()
{
    close();
}
