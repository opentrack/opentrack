/*******************************************************************************
* FaceTrackNoIR         This program is a private project of the some enthusiastic
*                                       gamers from Holland, who don't like to pay much for
*                                       head-tracking.
*
* Copyright (C) 2011    Wim Vriend (Developing)
*                                               Ron Hendriks (Researching and Testing)
*
* Homepage
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 3 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program; if not, see <http://www.gnu.org/licenses/>.
*********************************************************************************/
#include "ui.h"
#include "opentrack/tracker.h"
#include "opentrack/options.hpp"
#include "ftnoir_tracker_pt/ftnoir_tracker_pt.h"
#include "ftnoir_filter_accela/ftnoir_filter_accela.h"
#include <QFileDialog>
#include <QFileInfo>

#ifndef _WIN32
#   include <unistd.h>
#else
#   include <windows.h>
#endif

MainWindow::MainWindow() :
    pose_update_timer(this),
    kbd_quit(QKeySequence("Ctrl+Q"), this),
    no_feed_pixmap(":/images/no-feed.png")
{
    ui.setupUi(this);

    setFixedSize(size());

    updateButtonState(false, false);
    ui.video_frame_label->setPixmap(no_feed_pixmap);

    connect(ui.btnLoad, SIGNAL(clicked()), this, SLOT(open()));
    connect(ui.btnSave, SIGNAL(clicked()), this, SLOT(save()));
    connect(ui.btnSaveAs, SIGNAL(clicked()), this, SLOT(saveAs()));

    connect(ui.btnEditCurves, SIGNAL(clicked()), this, SLOT(showCurveConfiguration()));
    connect(ui.btnShortcuts, SIGNAL(clicked()), this, SLOT(showKeyboardShortcuts()));
    connect(ui.btnShowServerControls, SIGNAL(clicked()), this, SLOT(showProtocolSettings()));

    modules.filters().push_front(std::make_shared<dylib>("", dylib::Filter));

    for (auto x : modules.protocols())
        ui.iconcomboProtocol->addItem(x->icon, x->name);

    fill_profile_combobox();

    tie_setting(s.protocol_dll, ui.iconcomboProtocol);

    connect(ui.btnStartTracker, SIGNAL(clicked()), this, SLOT(startTracker()));
    connect(ui.btnStopTracker, SIGNAL(clicked()), this, SLOT(stopTracker()));
    connect(ui.iconcomboProfile, SIGNAL(currentIndexChanged(int)), this, SLOT(profileSelected(int)));

    connect(&pose_update_timer, SIGNAL(timeout()), this, SLOT(showHeadPose()));
    connect(&kbd_quit, SIGNAL(activated()), this, SLOT(exit()));
    kbd_quit.setEnabled(true);
    
    connect(&det_timer, SIGNAL(timeout()), this, SLOT(maybe_start_profile_from_executable()));
    det_timer.start(1000);

    ensure_tray();
}

MainWindow::~MainWindow()
{
    if (tray)
        tray->hide();
    stopTracker();
    save();
}

void MainWindow::set_working_directory()
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());
}

void MainWindow::open() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    QString dir_path = QFileInfo(group::ini_pathname()).absolutePath();
    QString fileName = dialog.getOpenFileName(
                this,
                tr("Open the settings file"),
                dir_path,
                tr("Settings file (*.ini);;All Files (*)"));
    set_working_directory();
    
    if (! fileName.isEmpty() ) {
        {
            QSettings settings(group::org);
            settings.setValue(group::filename_key, remove_app_path(fileName));
        }
        fill_profile_combobox();
        load_settings();
    }
}

void MainWindow::save_mappings() {
    pose.save_mappings();
}

#if defined(__unix) || defined(__linux) || defined(__APPLE__)
#   include <unistd.h>
#endif

void MainWindow::save() {
    s.b->save();
    save_mappings();
    mem<QSettings> settings = group::ini_file();
    settings->sync();

#if defined(__unix) || defined(__linux)
    QString currentFile = group::ini_pathname();
    QByteArray bytes = QFile::encodeName(currentFile);
    const char* filename_as_asciiz = bytes.constData();

    if (access(filename_as_asciiz, R_OK | W_OK))
    {
        QMessageBox::warning(this, "Something went wrong", "Check permissions and ownership for your .ini file!", QMessageBox::Ok, QMessageBox::NoButton);
    }
#endif
}

void MainWindow::saveAs()
{
    QString oldFile = group::ini_pathname();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"),
                                                    oldFile,
                                                    tr("Settings file (*.ini);;All Files (*)"));
    set_working_directory();
    
    if (fileName.isEmpty())
        return;
    
    (void) QFile::remove(fileName);
    
    {
        (void) QFile::copy(oldFile, fileName);
        QSettings settings(group::org);
        settings.setValue (group::filename_key, remove_app_path(fileName));
    }
    
    save();
    fill_profile_combobox();
}

void MainWindow::load_mappings() {
    pose.load_mappings();
}

void MainWindow::load_settings() {
    s.b->reload();
    load_mappings();
}

extern "C" volatile const char* opentrack_version;

void MainWindow::fill_profile_combobox()
{
     QStringList ini_list = group::ini_list();
     QString current = QFileInfo(group::ini_pathname()).fileName();
     setWindowTitle(QStringLiteral("TrackHat ") + QString( const_cast<const char*>(opentrack_version) + QStringLiteral(" :: ")) + current);
     ui.iconcomboProfile->clear();
     for (auto x : ini_list)
         ui.iconcomboProfile->addItem(QIcon(":/images/settings16.png"), x);
     ui.iconcomboProfile->setCurrentText(current);
}

void MainWindow::updateButtonState(bool running, bool inertialp)
{
    bool not_running = !running;
    ui.iconcomboProfile->setEnabled ( not_running );
    ui.btnStartTracker->setEnabled ( not_running );
    ui.btnStopTracker->setEnabled ( running );
    ui.iconcomboProtocol->setEnabled ( not_running );
    ui.video_frame_label->setVisible(not_running || inertialp);
    ui.btnSaveAs->setEnabled(not_running);
    ui.btnLoad->setEnabled(not_running);
}

void MainWindow::bindKeyboardShortcuts()
{
    if (work)
        work->reload_shortcuts();
    ensure_tray();
}

void MainWindow::startTracker() {
    s.b->save();
    load_settings();
    bindKeyboardShortcuts();

    // tracker dtor needs run first
    work = nullptr;

    libs = SelectedLibraries(ui.video_frame, std::make_shared<Tracker_PT>(), current_protocol(), std::make_shared<FTNoIR_Filter>());
    
    work = std::make_shared<Work>(s, pose, libs, this, winId());

    {
        double p[6] = {0,0,0, 0,0,0};
        display_pose(p, p);
    }

    if (!libs.correct)
    {
        QMessageBox::warning(this, "Library load error",
                             "One of libraries failed to load. Check installation.",
                             QMessageBox::Ok,
                             QMessageBox::NoButton);
        return;
    }
    
    if (pProtocolDialog)
        pProtocolDialog->register_protocol(libs.pProtocol.get());
    
    pose_update_timer.start(50);

    // NB check valid since SelectedLibraries ctor called
    // trackers take care of layout state updates
    const bool is_inertial = ui.video_frame->layout() == nullptr;
    updateButtonState(true, is_inertial);
}

void MainWindow::stopTracker( ) {
    //ui.game_name->setText("Not connected");

    pose_update_timer.stop();
    ui.pose_display->rotateBy(0, 0, 0, 0, 0, 0);

    if (pProtocolDialog)
    {
        pProtocolDialog->unregister_protocol();
        pProtocolDialog = nullptr;
    }

    work = nullptr;
    libs = SelectedLibraries();

    {
        double p[6] = {0,0,0, 0,0,0};
        display_pose(p, p);
    }
    updateButtonState(false, false);
}

void MainWindow::display_pose(const double *mapped, const double *raw)
{
    ui.pose_display->rotateBy(mapped[Yaw], mapped[Pitch], mapped[Roll],
                              mapped[TX], mapped[TY], mapped[TZ]);

    if (mapping_widget)
        mapping_widget->update();

    double mapped_[6], raw_[6];

    for (int i = 0; i < 6; i++)
    {
        mapped_[i] = (int) mapped[i];
        raw_[i] = (int) raw[i];
    }

    ui.raw_x->display(raw_[TX]);
    ui.raw_y->display(raw_[TY]);
    ui.raw_z->display(raw_[TZ]);
    ui.raw_yaw->display(raw_[Yaw]);
    ui.raw_pitch->display(raw_[Pitch]);
    ui.raw_roll->display(raw_[Roll]);

    ui.pose_x->display(mapped_[TX]);
    ui.pose_y->display(mapped_[TY]);
    ui.pose_z->display(mapped_[TZ]);
    ui.pose_yaw->display(mapped_[Yaw]);
    ui.pose_pitch->display(mapped_[Pitch]);
    ui.pose_roll->display(mapped_[Roll]);
}

void MainWindow::showHeadPose()
{
    double mapped[6], raw[6];

    work->tracker->get_raw_and_mapped_poses(mapped, raw);

    display_pose(mapped, raw);

#if 0
    if (libs.pProtocol)
    {
        const QString name = libs.pProtocol->game_name();
        ui.game_name->setText(name);
    }
#endif
}

template<typename t>
mem<t> mk_dialog(mem<dylib> lib)
{
    if (lib && lib->Dialog)
    {
        auto dialog = mem<t>(reinterpret_cast<t*>(lib->Dialog()));
        dialog->setWindowFlags(Qt::Dialog);
        dialog->setFixedSize(dialog->size());
        return dialog;
    }

    return nullptr;
}

void MainWindow::showProtocolSettings() {
    if (pProtocolDialog && pProtocolDialog->isVisible())
    {
        pProtocolDialog->show();
        pProtocolDialog->raise();
    } else
    {
        auto dialog = mk_dialog<IProtocolDialog>(current_protocol());
        pProtocolDialog = dialog;
        if (libs.pProtocol != nullptr)
            dialog->register_protocol(libs.pProtocol.get());
        dialog->show();
        dialog->raise();
    }
}

void MainWindow::showKeyboardShortcuts() {
    if (shortcuts_widget && shortcuts_widget->isVisible())
    {
        shortcuts_widget->show();
        shortcuts_widget->raise();
    }
    else
    {
        shortcuts_widget = std::make_shared<OptionsDialog>(static_cast<State&>(*this));
        shortcuts_widget->setWindowFlags(Qt::Dialog);
        connect(shortcuts_widget.get(), SIGNAL(reload()), this, SLOT(bindKeyboardShortcuts()));
        shortcuts_widget->show();
        shortcuts_widget->raise();
    }
}

void MainWindow::showCurveConfiguration() {
    if (mapping_widget && mapping_widget->isVisible())
    {
        mapping_widget->show();
        mapping_widget->raise();
    }
    else
    {
        mapping_widget = std::make_shared<MapWidget>(pose, s);
        mapping_widget->setWindowFlags(Qt::Dialog);
        mapping_widget->show();
    }
}

void MainWindow::exit() {
    QCoreApplication::exit(0);
}

QString MainWindow::remove_app_path(const QString full_path)
{
    QFileInfo path_info(full_path);
    QString path = path_info.absolutePath();
    
    QFileInfo app_path(QCoreApplication::applicationDirPath());
    QString app_prefix(app_path.absoluteFilePath());
    
    if (path == app_prefix)
    {
        path = ".";
    }
    else if (path.startsWith(app_prefix + "/"))
    {
        path = "./" + path.mid(app_prefix.size() + 1);
    }
    
    return path + "/" + path_info.fileName();
}

void MainWindow::profileSelected(int index)
{
    if (index == -1)
        return;
    
    {
        QSettings settings(group::org);
        settings.setValue (group::filename_key, remove_app_path(QFileInfo(group::ini_pathname()).absolutePath() + "/" +
                                                                ui.iconcomboProfile->itemText(index)));
    }
    load_settings();
}

void MainWindow::shortcutRecentered()
{
    qDebug() << "Center";
    if (work)
        work->tracker->center();
}

void MainWindow::shortcutToggled()
{
    qDebug() << "Toggle";
    if (work)
        work->tracker->toggle_enabled();
}

void MainWindow::shortcutZeroed()
{
    qDebug() << "Zero";
    if (work)
        work->tracker->zero();
}

void MainWindow::ensure_tray()
{
    if (tray)
        tray->hide();
    tray = nullptr;
    if (s.tray_enabled)
    {
        tray = std::make_shared<QSystemTrayIcon>(this);
        tray->setIcon(QIcon(":/images/facetracknoir.png"));
        tray->show();
        connect(tray.get(), SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(restore_from_tray(QSystemTrayIcon::ActivationReason)));
    }
}

void MainWindow::restore_from_tray(QSystemTrayIcon::ActivationReason)
{
    show();
    setWindowState(Qt::WindowNoState);
}

void MainWindow::changeEvent(QEvent* e)
{
    if (s.tray_enabled && e->type() == QEvent::WindowStateChange && (windowState() & Qt::WindowMinimized))
    {
        if (!tray)
            ensure_tray();
        hide();
    }
    QMainWindow::changeEvent(e);
}

void MainWindow::maybe_start_profile_from_executable()
{
    if (!work)
    {
        QString prof;
        if (det.config_to_start(prof))
        {
            ui.iconcomboProfile->setCurrentText(prof);
            startTracker();
        }
    }
    else
    {
        if (det.should_stop())
            stopTracker();
    }
}

void MainWindow::set_profile(const QString &profile)
{
    QSettings settings(group::org);
    settings.setValue(group::filename_key, MainWindow::remove_app_path(profile));
}
