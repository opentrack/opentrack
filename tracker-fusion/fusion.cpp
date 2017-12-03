/* Copyright (c) 2017 Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "compat/ndebug-guard.hpp"
#include "fusion.h"

#include "opentrack-library-path.h"

#include <QDebug>
#include <QMessageBox>
#include <QApplication>

static const QString own_name = QStringLiteral("fusion");

static auto get_modules()
{
    return Modules(OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH);
}

fusion_tracker::fusion_tracker()
{
}

fusion_tracker::~fusion_tracker()
{
    // CAVEAT order matters
    rot_tracker = nullptr;
    pos_tracker = nullptr;

    rot_dylib = nullptr;
    pos_dylib = nullptr;

    if (other_frame)
    {
        if (other_frame->layout())
            delete other_frame->layout();
        other_frame = nullptr;
    }
}

const QString& fusion_tracker::caption()
{
    static const QString caption = tr("Fusion tracker");
    return caption;
}

module_status fusion_tracker::start_tracker(QFrame* frame)
{
    assert(!rot_tracker && !pos_tracker);
    assert(!rot_dylib && !pos_dylib);

    QString err;
    module_status status;

    fusion_settings s;
    const QString rot_tracker_name = s.rot_tracker_name().toString();
    const QString pos_tracker_name = s.pos_tracker_name().toString();

    assert(rot_tracker_name != own_name);
    assert(pos_tracker_name != own_name);

    if (rot_tracker_name.isEmpty() || pos_tracker_name.isEmpty())
    {
        err = tr("Trackers not selected.");
        goto end;
    }

    if (rot_tracker_name == pos_tracker_name)
    {
        err = tr("Select different trackers for rotation and position.");
        goto end;
    }

    {
        Modules libs = get_modules();

        for (auto& t : libs.trackers())
        {
            if (t->module_name == rot_tracker_name)
            {
                assert(!rot_dylib);
                rot_dylib = t;
            }

            if (t->module_name == pos_tracker_name)
            {
                assert(!pos_dylib);
                pos_dylib = t;
            }
        }
    }

    if (!rot_dylib || !pos_dylib)
        goto end;

    rot_tracker = make_dylib_instance<ITracker>(rot_dylib);
    pos_tracker = make_dylib_instance<ITracker>(pos_dylib);

    status = pos_tracker->start_tracker(frame);

    if (!status.is_ok())
    {
        err = pos_dylib->name + QStringLiteral(":\n    ") + status.error;
        goto end;
    }

    if (frame->layout() == nullptr)
    {
        status = rot_tracker->start_tracker(frame);
        other_frame = nullptr;
        if (!status.is_ok())
        {
            err = rot_dylib->name + QStringLiteral(":\n    ") + status.error;
            goto end;
        }
    }
    else
    {
        other_frame->setVisible(false);
        other_frame->setFixedSize(320, 240); // XXX magic frame size

        rot_tracker->start_tracker(other_frame.get());

        if (other_frame->layout() == nullptr)
            other_frame = nullptr;
        else
            other_frame->hide();

    }

end:
    if (!err.isEmpty())
        return error(err);
    else
        return status_ok();
}

void fusion_tracker::data(double *data)
{
    if (pos_tracker && rot_tracker)
    {
        rot_tracker->data(rot_tracker_data);
        pos_tracker->data(pos_tracker_data);

        for (unsigned k = 0; k < 3; k++)
            data[k] = pos_tracker_data[k];
        for (unsigned k = 3; k < 6; k++)
            data[k] = rot_tracker_data[k];
    }
}

fusion_dialog::fusion_dialog()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    ui.rot_tracker->addItem("");
    ui.pos_tracker->addItem("");

    Modules libs = get_modules();

    for (auto& m : libs.trackers())
    {
        if (m->module_name == own_name)
            continue;

        ui.rot_tracker->addItem(m->icon, m->name, QVariant(m->module_name));
        ui.pos_tracker->addItem(m->icon, m->name, QVariant(m->module_name));
    }

    ui.rot_tracker->setCurrentIndex(0);
    ui.pos_tracker->setCurrentIndex(0);

    tie_setting(s.rot_tracker_name, ui.rot_tracker);
    tie_setting(s.pos_tracker_name, ui.pos_tracker);
}

void fusion_dialog::doOK()
{
    const int rot_idx = ui.rot_tracker->currentIndex() - 1;
    const int pos_idx = ui.pos_tracker->currentIndex() - 1;

    if (rot_idx == -1 || pos_idx == -1 || rot_idx == pos_idx)
    {
        QMessageBox::warning(this,
                             fusion_tracker::caption(),
                             tr("Fusion tracker only works when distinct trackers are selected "
                                "for rotation and position."),
                             QMessageBox::Close);
    }

    s.b->save();
    close();
}

void fusion_dialog::doCancel()
{
    close();
}

fusion_settings::fusion_settings() :
    opts("fusion-tracker"),
    rot_tracker_name(b, "rot-tracker", ""),
    pos_tracker_name(b, "pos-tracker", "")
{
}

OPENTRACK_DECLARE_TRACKER(fusion_tracker, fusion_dialog, fusion_metadata)
