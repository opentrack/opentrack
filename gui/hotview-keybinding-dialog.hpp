#pragma once

#include "export.hpp"
#include "logic/hotview.hpp"

#include <QWidget>

OTR_GUI_EXPORT bool prompt_hotview_key_binding(QWidget* parent, hotview_key& out);
