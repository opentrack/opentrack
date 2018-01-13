#pragma once

#include "export.hpp"

#include <functional>
#include <QWidget>

int OTR_GUI_EXPORT otr_main(int argc, char** argv, std::function<QWidget*()> make_main_window);

template<typename F>
auto run_application(int argc, char** argv, F&& fun)
{
    return otr_main(argc, argv, fun);
}
