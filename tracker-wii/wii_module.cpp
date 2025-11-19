/*
* Copyright (c) 2017-2018 Wei Shuai <cpuwolf@gmail.com>
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*/
#include "ftnoir_tracker_pt.h"
#include "api/plugin-api.hpp"

#include "wii_camera.h"
#include "wii_frame.hpp"
#include "wii_point_extractor.h"
#include "ftnoir_tracker_pt_dialog.h"

#include "pt-api.hpp"

#include "compat/linkage-macros.hpp"

#include <memory>

static const QString module_name = "tracker-wii-pt";

using namespace pt_module;

struct wii_pt_module_traits final : pt_runtime_traits
{
    pointer<pt_camera> make_camera() const override
    {
        return pointer<pt_camera>(new WIICamera(module_name));
    }

    pointer<pt_point_extractor> make_point_extractor() const override
    {
        return pointer<pt_point_extractor>(new WIIPointExtractor(module_name));
    }

    QString get_module_name() const override
    {
        return module_name;
    }

    pointer<pt_frame> make_frame() const override
    {
        return pointer<pt_frame>(new WIIFrame);
    }

    pointer<pt_preview> make_preview(int w, int h) const override
    {
        return pointer<pt_preview>(new WIIPreview(w, h));
    }
};

struct wii_tracker_pt : Tracker_PT
{
    wii_tracker_pt() : Tracker_PT(pointer<pt_runtime_traits>(new wii_pt_module_traits))
    {
    }
};


struct wii_dialog_pt : TrackerDialog_PT
{
    wii_dialog_pt();
};

struct OTR_GENERIC_EXPORT wii_metadata_pt : Metadata
{
    Q_OBJECT

    QString name() override;
    QIcon icon() override;
};

QString wii_metadata_pt::name() { return tr("WiiPointTracker 1.1"); }
QIcon wii_metadata_pt::icon() { return QIcon(":/Resources/wii.png"); }

// ns pt_module

using namespace pt_module;



wii_dialog_pt::wii_dialog_pt() : TrackerDialog_PT(module_name)
{
	ui.camera_settings_groupbox->hide();
	ui.groupBox_2->hide();
}

OPENTRACK_DECLARE_TRACKER(wii_tracker_pt, wii_dialog_pt, wii_metadata_pt)
