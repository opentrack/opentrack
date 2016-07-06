#pragma once

#ifndef OPENTRACK_OPTIONS_EXTERN_TEMPLATES
#   define OPENTRACK_OPTIONS_EXTERN_TEMPLATES
#   error "define OPENTRACK_OPTIONS_EXTERN_TEMPLATES before including"
#endif

#include "export.hpp"

namespace options {
    OPENTRACK_OPTIONS_EXTERN_TEMPLATES template OPENTRACK_COMPAT_EXPORT void tie_setting<int, QComboBox>(value<int>& v, QComboBox* cb);
    OPENTRACK_OPTIONS_EXTERN_TEMPLATES template OPENTRACK_COMPAT_EXPORT void tie_setting<QString, QComboBox>(value<QString>& v, QComboBox* cb);
    OPENTRACK_OPTIONS_EXTERN_TEMPLATES template OPENTRACK_COMPAT_EXPORT void tie_setting<bool, QCheckBox>(value<bool>& v, QCheckBox* cb);
    OPENTRACK_OPTIONS_EXTERN_TEMPLATES template OPENTRACK_COMPAT_EXPORT void tie_setting<double, QDoubleSpinBox>(value<double>& v, QDoubleSpinBox* dsb);
    OPENTRACK_OPTIONS_EXTERN_TEMPLATES template OPENTRACK_COMPAT_EXPORT void tie_setting<int, QSpinBox>(value<int>& v, QSpinBox* sb);
    OPENTRACK_OPTIONS_EXTERN_TEMPLATES template OPENTRACK_COMPAT_EXPORT void tie_setting<int, QSlider>(value<int>& v, QSlider* sl);
    OPENTRACK_OPTIONS_EXTERN_TEMPLATES template OPENTRACK_COMPAT_EXPORT void tie_setting<QString, QLineEdit>(value<QString>& v, QLineEdit* le);
    OPENTRACK_OPTIONS_EXTERN_TEMPLATES template OPENTRACK_COMPAT_EXPORT void tie_setting<QString, QLabel>(value<QString>& v, QLabel* lb);
    OPENTRACK_OPTIONS_EXTERN_TEMPLATES template OPENTRACK_COMPAT_EXPORT void tie_setting<int, QTabWidget>(value<int>& v, QTabWidget* t);
    OPENTRACK_OPTIONS_EXTERN_TEMPLATES template OPENTRACK_COMPAT_EXPORT void tie_setting<slider_value, QSlider>(value<slider_value>& v, QSlider* w);
}
