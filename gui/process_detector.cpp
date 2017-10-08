/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "process_detector.h"
#include "main-window.hpp"
#include "compat/process-list.hpp"
#include <QList>
#include <QFileDialog>
#include <QComboBox>
#include <QString>
#include <QHash>
#include <QPushButton>
#include <QSettings>

void settings::set_game_list(const QString &game_list)
{
    group::with_global_settings_object([&](QSettings& settings) {
        settings.setValue("executable-list", game_list);
    });
}

QString settings::get_game_list()
{
    return group::with_global_settings_object([&](QSettings& settings) {
        return settings.value("executable-list").toString();
    });
}

bool settings::is_enabled()
{
    return group::with_global_settings_object([&](QSettings& settings) {
        return settings.value("executable-detector-enabled", false).toBool();
    });
}

void settings::set_is_enabled(bool enabled)
{
    group::with_global_settings_object([&](QSettings& settings) {
        settings.setValue("executable-detector-enabled", enabled);
    });
}

QHash<QString, QString> settings::split_process_names()
{
    QHash<QString, QString> ret;
    QString str = get_game_list();
    QStringList pairs = str.split('|');
    for (auto pair : pairs)
    {
        QList<QString> tmp = pair.split(':');
        if (tmp.count() != 2)
            continue;
        ret[tmp[0]] = tmp[1];
    }
    return ret;
}

void BrowseButton::browse()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    QString dir_path = QFileInfo(group::ini_pathname()).absolutePath();
    QString filename = dialog.getOpenFileName(
                this,
                tr("Set executable name"),
                dir_path,
                tr("Executable (*.exe);;All Files (*)"));
    MainWindow::set_working_directory();
    filename = QFileInfo(filename).fileName();
    if (!filename.isNull())
        twi->setText(filename);
}

int process_detector::add_row(QString exe_name, QString profile)
{
    int i = ui.tableWidget->rowCount();
    ui.tableWidget->insertRow(i);

    QComboBox* cb = new QComboBox();
    cb->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    cb->addItem("");
    cb->addItems(group::ini_list());
    ui.tableWidget->setCellWidget(i, 1, cb);

    QTableWidgetItem* twi = new QTableWidgetItem(exe_name);
    ui.tableWidget->setItem(i, 0, twi);

    {
        BrowseButton* b = new BrowseButton(twi);
        b->setText("...");
        QObject::connect(b, SIGNAL(clicked()), b, SLOT(browse()));
        ui.tableWidget->setCellWidget(i, 2, b);
    }

    cb->setCurrentText(profile);

    return i;
}

void process_detector::add_items()
{
    auto names = s.split_process_names();
    ui.tableWidget->clearContents();
    auto keys = names.keys();
    std::sort(keys.begin(), keys.end());
    for (auto n : keys)
        add_row(n, names[n]);
}

process_detector::process_detector(QWidget* parent) : QWidget(parent)
{
    ui.setupUi(this);
    connect(ui.add, SIGNAL(clicked()), this, SLOT(add()));
    connect(ui.remove, SIGNAL(clicked()), this, SLOT(remove()));

    add_items();

    QResizeEvent e(ui.tableWidget->size(), ui.tableWidget->size());
    ui.tableWidget->resizeEvent(&e);

    settings s;

    ui.enabled->setChecked(s.is_enabled());
}

void process_detector::save()
{
    QString str;

    for (int i = 0; i < ui.tableWidget->rowCount(); i++)
    {
        auto exe = ui.tableWidget->item(i, 0)->text();
        auto profile = reinterpret_cast<QComboBox*>(ui.tableWidget->cellWidget(i, 1))->currentText();
        str += "|" + exe + ":" + profile;
    }

    s.set_game_list(str);
    s.set_is_enabled(ui.enabled->isChecked());
}

void process_detector::revert()
{
}

void process_detector::add()
{
    add_row();
}

void process_detector::remove()
{
    int r = ui.tableWidget->currentRow();
    if (r != -1)
        ui.tableWidget->removeRow(r);
}

bool process_detector_worker::should_stop()
{
    if (last_exe_name == "")
        return false;

    settings s;

    if (!s.is_enabled())
    {
        last_exe_name = "";
        return false;
    }

    QStringList exe_list = get_all_executable_names();

    if (exe_list.contains(last_exe_name))
        return false;

    last_exe_name = "";

    return true;
}

bool process_detector_worker::config_to_start(QString& str)
{
    settings s;
    if (!s.is_enabled())
    {
        last_exe_name = "";
        return false;
    }

    auto filenames = s.split_process_names();
    QStringList exe_list = get_all_executable_names();

    // assuming manual stop by user button click.
    // don't automatically start again while the same process is running.
    if (last_exe_name != "" && exe_list.contains(last_exe_name))
        return false;
    // it's gone, we can start automatically again
    last_exe_name = "";

    for (auto& name : exe_list)
    {
        if (filenames.contains(name))
        {
            last_exe_name = name;
            str = filenames[name];
            return str != "";
        }
    }

    return false;
}
