#include "window.hpp"

#include <QApplication>

void force_trackmouse_settings();

void window::closeEvent(QCloseEvent* e)
{
    e->accept();
    QApplication::exit(0);
}

window::window() : QMainWindow()
{
    ui.setupUi(this);
    setAttribute(Qt::WA_QuitOnClose);

    force_trackmouse_settings();

    show();
}

