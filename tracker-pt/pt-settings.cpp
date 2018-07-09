#include "pt-settings.hpp"

namespace pt_settings_detail {

pt_settings::pt_settings(const QString& name) : opts(name) {}
pt_settings::~pt_settings() = default;

} // ns pt_settings_detail
