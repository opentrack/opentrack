#pragma once

#ifndef BUILD_QXT_MINI
#   error "internal header"
#endif

#ifndef __APPLE__

#include <Qt>
#include <QDebug>
#include <X11/Xlib.h>

unsigned qt_key_to_x11(Display* disp, Qt::Key k);

#endif
