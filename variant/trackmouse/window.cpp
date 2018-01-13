#include "window.hpp"

#include <QApplication>

void window::closeEvent(QCloseEvent* e)
{
    e->accept();
    QApplication::exit(0);
}

window::window() : QMainWindow()
{
    ui.setupUi(this);
    setAttribute(Qt::WA_QuitOnClose);

    show();
}

