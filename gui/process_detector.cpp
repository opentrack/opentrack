/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "process_detector.h"
#include "options/options.hpp"
#include "compat/process-list.hpp"
#include "compat/base-path.hpp"

#include <QList>
#include <QFileDialog>
#include <QComboBox>
#include <QString>
#include <QHash>
#include <QPushButton>
#include <QSettings>
#include <QtEvents>

static constexpr auto RECORD_SEPARATOR = QChar(char(0x1e));  // RS ^]
static constexpr auto UNIT_SEPARATOR = QChar(char(0x1f));    // US ^_

using namespace options;
using namespace options::globals;

void proc_detector_settings::set_game_list(const QString &game_list)
{
    with_global_settings_object([&](QSettings& settings) {
        settings.setValue("executable-list", game_list);
        mark_global_ini_modified();
    });
}

QString proc_detector_settings::get_game_list()
{
    return with_global_settings_object([&](QSettings& settings) {
        return settings.value("executable-list").toString();
    });
}

bool proc_detector_settings::is_enabled()
{
    return with_global_settings_object([&](QSettings& settings) {
        return settings.value("executable-detector-enabled", false).toBool();
    });
}

void proc_detector_settings::set_is_enabled(bool enabled)
{
    with_global_settings_object([&](QSettings& settings) {
        settings.setValue("executable-detector-enabled", enabled);
        mark_global_ini_modified();
    });
}

#ifdef __GNUG__
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

QHash<QString, QString> proc_detector_settings::split_process_names()
{
    QString str = get_game_list();
    constexpr auto split_flag =
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        Qt::SkipEmptyParts;
#else
        QString::SkipEmptyParts;
#endif
    QStringList pairs = str.split(RECORD_SEPARATOR, split_flag);
    QHash<QString, QString> ret;
    ret.reserve(pairs.size() * 2);
    for (auto const& pair : pairs)
    {
        QStringList tmp = pair.split(UNIT_SEPARATOR);
        if (tmp.count() != 2)
            continue;
        if (tmp[0].contains(UNIT_SEPARATOR) || tmp[0].contains(RECORD_SEPARATOR))
            continue;
        if (tmp[1].contains(UNIT_SEPARATOR) || tmp[1].contains(RECORD_SEPARATOR))
            continue;
        if (tmp[0].isEmpty() || tmp[1].isEmpty())
            continue;
        ret[tmp[0]] = tmp[1];
    }
    return ret;
}

void BrowseButton::browse()
{
    QString dir_path = QFileInfo(ini_pathname()).absolutePath();
    QString filename = QFileDialog::getOpenFileName(
                           this,
                           tr("Set executable name"),
                           dir_path,
                           tr("Executable (*.exe);;All Files (*)"));
    QDir::setCurrent(OPENTRACK_BASE_PATH);
    filename = QFileInfo(filename).fileName();
    if (!filename.isNull())
        twi->setText(filename);
}

int process_detector::add_row(QString const& exe_name, QString const& profile)
{
    int i = ui.tableWidget->rowCount();
    ui.tableWidget->insertRow(i);

    QComboBox* cb = new QComboBox();
    cb->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    cb->addItems(ini_list());
    cb->setCurrentText(ini_filename());
    ui.tableWidget->setCellWidget(i, 1, cb);

    QTableWidgetItem* twi = new QTableWidgetItem(exe_name);
    ui.tableWidget->setItem(i, 0, twi);

    {
        BrowseButton* b = new BrowseButton(twi);
        b->setText("...");
        QObject::connect(b, &BrowseButton::clicked, b, &BrowseButton::browse);
        ui.tableWidget->setCellWidget(i, 2, b);
    }

    cb->setCurrentText(profile);

    return i;
}

void process_detector::load_rows()
{
    for (int k = ui.tableWidget->size().height() - 1; k >= 0; k--)
        ui.tableWidget->removeRow(k);
    auto names = s.split_process_names();
    auto keys = names.keys();
    std::sort(keys.begin(), keys.end());
    for (auto const& n : keys)
        add_row(n, names[n]);
}

process_detector::process_detector(QWidget* parent) : QWidget(parent)
{
    ui.setupUi(this);
    connect(ui.add, SIGNAL(clicked()), this, SLOT(add()));
    connect(ui.remove, SIGNAL(clicked()), this, SLOT(remove()));

    load_rows();

    QResizeEvent e(ui.tableWidget->size(), ui.tableWidget->size());
    ui.tableWidget->resizeEvent(&e);
    ui.enabled->setChecked(s.is_enabled());
}

void process_detector::save()
{
    QString str;

    for (int i = 0; i < ui.tableWidget->rowCount(); i++)
    {
        auto exe = ui.tableWidget->item(i, 0)->text();
        auto widget = qobject_cast<QComboBox*>(ui.tableWidget->cellWidget(i, 1));
        if (!widget)
            continue;
        auto profile = widget->currentText();
        str += RECORD_SEPARATOR + exe + UNIT_SEPARATOR + profile;
    }

    s.set_game_list(str);
    s.set_is_enabled(ui.enabled->isChecked());
}

void process_detector::revert()
{
    load_rows();
    ui.enabled->setChecked(s.is_enabled());
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
    if (last_exe_name == QString())
        return false;

    proc_detector_settings s;

    if (!s.is_enabled())
    {
        last_exe_name = QString{};
        return false;
    }

    QStringList exe_list = get_all_executable_names();

    if (exe_list.contains(last_exe_name))
        return false;

    last_exe_name = QString{};

    return true;
}

bool process_detector_worker::profile_to_start(QString& str)
{
    proc_detector_settings s;
    if (!s.is_enabled())
    {
        last_exe_name = QString{};
        return false;
    }

    auto filenames = s.split_process_names();
    if (filenames.isEmpty())
        return false;

    QStringList exe_list = get_all_executable_names();

    // assuming manual stop by user button click.
    // don't automatically start again while the same process is running.
    if (!last_exe_name.isEmpty() && exe_list.contains(last_exe_name))
        return false;
    // it's gone, we can start automatically again
    last_exe_name = QString();

    for (auto&& name : exe_list)
        if (filenames.contains(name))
        {
            str = filenames[name];
            last_exe_name = name;
            return str != QString{};
        }

    return false;
}
