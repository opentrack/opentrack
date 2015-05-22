#pragma once

#include <QObject>
#include <QWidget>
#include <QTableWidget>
#include <QResizeEvent>

#include "opentrack/options.hpp"
using namespace options;

class FancyTable : public QTableWidget
{
    Q_OBJECT
public:
    void resizeEvent(QResizeEvent* e) override
    {
        QTableView::resizeEvent(e);
        int w = width();
        setColumnWidth(2, 32);
        w -= 48;
        setColumnWidth(0, w / 2);
        setColumnWidth(1, w / 2);
    }
public:
    FancyTable(QWidget* parent = nullptr) : QTableWidget(parent) {}
};

struct settings
{
    QHash<QString, QString> split_process_names();
    QString get_game_list();
    void set_game_list(const QString& game_list);
    bool is_enabled();
    void set_is_enabled(bool enabled);
};

#include "ui_process_widget.h"

class process_detector : public QWidget
{
    Q_OBJECT
    
    Ui_Dialog ui;
    settings s;
    
    int add_row(QString exe_name = "...", QString profile = "");
    void add_items();
public:
    process_detector(QWidget* parent = nullptr);
public slots:
    void save();
    void revert();
private slots:
    void add();
    void remove();
};

class BrowseButton : public QPushButton
{
    Q_OBJECT
    QTableWidgetItem* twi;
public:
    BrowseButton(QTableWidgetItem* twi) : twi(twi)
    {}
public slots:
    void browse();
};

#ifdef _WIN32

class process_detector_worker : QObject
{
    Q_OBJECT
    settings s;
    QString last_exe_name;
public:
    bool config_to_start(QString& s);
    bool should_stop();
};

#else

class process_detector_worker : QObject
{
    Q_OBJECT
public:
    bool config_to_start(QString&)
    {
        return false;
    }
};

#endif
