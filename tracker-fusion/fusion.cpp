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

static const QString own_name = fusion_metadata().name();

fusion_tracker::fusion_tracker() :
    rot_tracker_data{},
    pos_tracker_data{},
    other_frame(new QFrame)
{
}

fusion_tracker::~fusion_tracker()
{
    rot_tracker = nullptr;
    pos_tracker = nullptr;
    rot_dylib = nullptr;
    pos_dylib = nullptr;

    if (other_frame)
    {
        assert(other_frame->layout());
        delete other_frame->layout();
        other_frame = nullptr;
    }
}

void fusion_tracker::start_tracker(QFrame* frame)
{
    assert(!rot_tracker && !pos_tracker);
    assert(!rot_dylib && !pos_dylib);

    fusion_settings s;
    const QString rot_tracker_name = s.rot_tracker_name;
    const QString pos_tracker_name = s.pos_tracker_name;

    assert(rot_tracker_name != own_name);
    assert(pos_tracker_name != own_name);

    if (rot_tracker_name.isEmpty() || pos_tracker_name.isEmpty())
    {
        QMessageBox::warning(nullptr,
                             "Fusion tracker", "Select rotation and position trackers.",
                             QMessageBox::Close);
        other_frame = nullptr;
        return;
    }

    if (rot_tracker_name == pos_tracker_name)
    {
        QMessageBox::warning(nullptr,
                            "Fusion tracker", "Select different trackers for rotation and position.",
                            QMessageBox::Close);
        other_frame = nullptr;
        return;
    }

    for (auto& t : modules.trackers())
    {
        if (t->name == rot_tracker_name)
        {
            assert(!rot_dylib);
            rot_dylib = t;
        }
        if (t->name == pos_tracker_name)
        {
            assert(!pos_dylib);
            pos_dylib = t;
        }
    }

    rot_tracker = make_dylib_instance<ITracker>(rot_dylib);
    pos_tracker = make_dylib_instance<ITracker>(pos_dylib);

    pos_tracker->start_tracker(frame);

    if (frame->layout() == nullptr)
    {
        rot_tracker->start_tracker(frame);
        other_frame = nullptr;
    }
    else
    {
        rot_tracker->start_tracker(other_frame.get());
        if (!other_frame->layout())
            other_frame = nullptr;
    }


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

    for (auto &m : modules.trackers())
    {
        if (m->name == own_name)
            continue;

        ui.rot_tracker->addItem(m->icon, m->name);
        ui.pos_tracker->addItem(m->icon, m->name);
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
                             "Fusion tracker",
                             "Fusion tracker only works when distinct trackers are selected "
                             "for rotation and position.",
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

has_modules::has_modules() :
    modules(OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH)
{
}


OPENTRACK_DECLARE_TRACKER(fusion_tracker, fusion_dialog, fusion_metadata)