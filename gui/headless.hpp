#pragma once

#include <QString>

// Run tracking without the main window. `profile` may be empty to use the
// profile already selected in the global settings. Returns the process
// exit code.
int run_headless(const QString& profile);
