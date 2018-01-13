#pragma once

#include "ui_window.h"
#include <QMainWindow>
#include <QCloseEvent>

class window : public QMainWindow
{
    Q_OBJECT

    Ui::window ui;

    void closeEvent(QCloseEvent* e) override;
public:
    window();

};
