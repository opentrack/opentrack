#pragma once

#include "export.hpp"

// macOS: hide the Dock icon and menu bar so a headless run behaves like a
// background agent. No-op on other platforms.
OTR_COMPAT_EXPORT
void set_background_app();
