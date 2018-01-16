#pragma once

#include "export.hpp"

#include <functional>
#include <QWidget>

int OTR_GUI_EXPORT otr_main(int argc, char** argv, std::function<QWidget*()> make_main_window);

// XXX TODO need split MainWindow into mixins each implementing part of the functionality

template<typename F>
auto run_application(int argc, char** argv, F&& fun)
{
    return otr_main(argc, argv, fun);
}
