#include "plugin-api.hpp"

void plugin_api::detail::BaseDialog::closeEvent(QCloseEvent*) { emit closing(); }
