#pragma once

#include "export.hpp"

// Prevent macOS from throttling the process while tracking is active.
OTR_COMPAT_EXPORT
void disable_appnap_start();

OTR_COMPAT_EXPORT
void disable_appnap_stop();
