#pragma once

#include "export.hpp"
#include "logic/snapview.hpp"

#include <QWidget>

OTR_GUI_EXPORT bool prompt_snapview_key_binding(QWidget* parent, snapview_key& out);
