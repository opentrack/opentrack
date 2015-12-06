/* Copyright (c) 2013-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "ui.h"
#include "opentrack/tracker.h"
#include "opentrack-compat/options.hpp"
#include "new_file_dialog.h"
#include <QFileDialog>
#include <QDesktopServices>

#ifndef _WIN32
#   include <unistd.h>
#else
#   include <windows.h>
#endif

MainWindow::MainWindow() :
    pose_update_timer(this),
    kbd_quit(QKeySequence("Ctrl+Q"), this),
    no_feed_pixmap(":/images/no-feed.png"),
    is_refreshing_profiles(false),
    keys_paused(false)
{
    ui.setupUi(this);

    setFixedSize(size());

    updateButtonState(false, false);
    ui.video_frame_label->setPixmap(no_feed_pixmap);

    connect(ui.btnEditCurves, SIGNAL(clicked()), this, SLOT(showCurveConfiguration()));
    connect(ui.btnShortcuts, SIGNAL(clicked()), this, SLOT(show_options_dialog()));
    connect(ui.btnShowEngineControls, SIGNAL(clicked()), this, SLOT(showTrackerSettings()));
    connect(ui.btnShowServerControls, SIGNAL(clicked()), this, SLOT(showProtocolSettings()));
    connect(ui.btnShowFilterControls, SIGNAL(clicked()), this, SLOT(showFilterSettings()));

    modules.filters().push_front(std::make_shared<dylib>("", dylib::Filter));

    for (auto x : modules.trackers())
        ui.iconcomboTrackerSource->addItem(x->icon, x->name);

    for (auto x : modules.protocols())
        ui.iconcomboProtocol->addItem(x->icon, x->name);

    for (auto x : modules.filters())
        ui.iconcomboFilter->addItem(x->icon, x->name);

    refresh_config_list();
    connect(&config_list_timer, SIGNAL(timeout()), this, SLOT(refresh_config_list()));
    config_list_timer.start(1000 * 3);

    tie_setting(s.tracker_dll, ui.iconcomboTrackerSource);
    tie_setting(s.protocol_dll, ui.iconcomboProtocol);
    tie_setting(s.filter_dll, ui.iconcomboFilter);

    connect(ui.iconcomboTrackerSource,
            &QComboBox::currentTextChanged,
            [&](QString) -> void { if (pTrackerDialog) pTrackerDialog = nullptr; save(); });

    connect(ui.iconcomboProtocol,
            &QComboBox::currentTextChanged,
            [&](QString) -> void { if (pProtocolDialog) pProtocolDialog = nullptr; save(); });

    connect(ui.iconcomboFilter,
            &QComboBox::currentTextChanged,
            [&](QString) -> void { if (pFilterDialog) pFilterDialog = nullptr; save(); });

    connect(ui.btnStartTracker, SIGNAL(clicked()), this, SLOT(startTracker()));
    connect(ui.btnStopTracker, SIGNAL(clicked()), this, SLOT(stopTracker()));
    connect(ui.iconcomboProfile, SIGNAL(currentTextChanged(QString)), this, SLOT(profileSelected(QString)));

    connect(&pose_update_timer, SIGNAL(timeout()), this, SLOT(showHeadPose()));
    connect(&kbd_quit, SIGNAL(activated()), this, SLOT(exit()));

    save_timer.setSingleShot(true);
    connect(&save_timer, SIGNAL(timeout()), this, SLOT(_save()));

    profile_menu.addAction("Create new empty config", this, SLOT(make_empty_config()));
    profile_menu.addAction("Create new copied config", this, SLOT(make_copied_config()));
    profile_menu.addAction("Open configuration directory", this, SLOT(open_config_directory()));
    ui.profile_button->setMenu(&profile_menu);

    kbd_quit.setEnabled(true);
    
    connect(&det_timer, SIGNAL(timeout()), this, SLOT(maybe_start_profile_from_executable()));
    det_timer.start(1000);

    ensure_tray();
    set_working_directory();

    if (!QFile(group::ini_pathname()).exists())
    {
        set_profile(OPENTRACK_DEFAULT_CONFIG);
        const auto pathname = group::ini_pathname();
        if (!QFile(pathname).exists())
        {
            QFile file(pathname);
            (void) file.open(QFile::ReadWrite);
        }
    }

    if (group::ini_directory() == "")
        QMessageBox::warning(this,
                             "Configuration not saved.",
                             "Can't create configuration directory! Expect major malfunction.",
                             QMessageBox::Ok, QMessageBox::NoButton);
    
    connect(this, &MainWindow::emit_start_tracker,
            this, [&]() -> void { if (keys_paused) return; qDebug() << "start tracker"; startTracker(); },
            Qt::QueuedConnection);
    
    connect(this, &MainWindow::emit_stop_tracker,
            this, [&]() -> void { if (keys_paused) return; qDebug() << "stop tracker"; stopTracker(); },
            Qt::QueuedConnection);
    
    connect(this, &MainWindow::emit_toggle_tracker,
            this, [&]() -> void { if (keys_paused) return; qDebug() << "toggle tracker"; if (work) stopTracker(); else startTracker(); },
            Qt::QueuedConnection);
    
    register_shortcuts();
    
    ui.btnStartTracker->setFocus();
}

void MainWindow::register_shortcuts()
{
    using t_shortcut = std::tuple<key_opts&, Shortcuts::fun>;
    
    std::vector<t_shortcut> keys {
        t_shortcut(s.key_start_tracking, [&]() -> void { emit_start_tracker(); }),
        t_shortcut(s.key_stop_tracking, [&]() -> void { emit_stop_tracker(); }),
        t_shortcut(s.key_toggle_tracking, [&]() -> void { emit_toggle_tracker(); }),
    };
    
    global_shortcuts.reload(keys);
    
    if (work)
        work->reload_shortcuts();
}

bool MainWindow::get_new_config_name_from_dialog(QString& ret)
{
    new_file_dialog dlg;
    dlg.exec();
    return dlg.is_ok(ret);
}

MainWindow::~MainWindow()
{
    maybe_save();

    if (tray)
        tray->hide();
    stopTracker();
}

void MainWindow::set_working_directory()
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());
}

void MainWindow::save_mappings() {
    pose.save_mappings();
}

void MainWindow::save()
{
    save_timer.stop();
    save_timer.start(5000);
}

void MainWindow::maybe_save()
{
    if (save_timer.isActive())
    {
        save_timer.stop();
        _save();
    }
}

void MainWindow::_save() {
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

void MainWindow::load_mappings() {
    pose.load_mappings();
}

void MainWindow::load_settings() {
    s.b->reload();
    load_mappings();
}

void MainWindow::make_empty_config()
{
    QString name;
    const QString dir = group::ini_directory();
    if (dir != "" && get_new_config_name_from_dialog(name))
    {
        QFile filename(dir + "/" + name);
        (void) filename.open(QFile::ReadWrite);
        refresh_config_list();
        ui.iconcomboProfile->setCurrentText(name);
    }
}

void MainWindow::make_copied_config()
{
    const QString dir = group::ini_directory();
    const QString cur = group::ini_pathname();
    QString name;
    if (cur != "" && dir != "" && get_new_config_name_from_dialog(name))
    {
        const QString new_name = dir + "/" + name;
        (void) QFile::remove(new_name);
        (void) QFile::copy(cur, new_name);
        refresh_config_list();
        ui.iconcomboProfile->setCurrentText(name);
    }
}

void MainWindow::open_config_directory()
{
    const QString path = group::ini_directory();
    if (path != "")
    {
        QDesktopServices::openUrl("file:///" + QDir::toNativeSeparators(path));
    }
}

extern "C" const char* opentrack_version;

void MainWindow::refresh_config_list()
{
    if (work)
        return;

    if (group::ini_list().size() == 0)
    {
        QFile filename(group::ini_directory() + "/" OPENTRACK_DEFAULT_CONFIG);
        (void) filename.open(QFile::ReadWrite);
    }

     QStringList ini_list = group::ini_list();
     set_title();
     QString current = group::ini_filename();
     is_refreshing_profiles = true;
     ui.iconcomboProfile->clear();
     for (auto x : ini_list)
         ui.iconcomboProfile->addItem(QIcon(":/images/settings16.png"), x);
     is_refreshing_profiles = false;
     ui.iconcomboProfile->setCurrentText(current);
}

void MainWindow::updateButtonState(bool running, bool inertialp)
{
    bool not_running = !running;
    ui.iconcomboProfile->setEnabled ( not_running );
    ui.btnStartTracker->setEnabled ( not_running );
    ui.btnStopTracker->setEnabled ( running );
    ui.iconcomboProtocol->setEnabled ( not_running );
    ui.iconcomboFilter->setEnabled ( not_running );
    ui.iconcomboTrackerSource->setEnabled(not_running);
    ui.video_frame_label->setVisible(not_running || inertialp);
    ui.profile_button->setEnabled(not_running);
}

void MainWindow::reload_options()
{
    if (work)
        work->reload_shortcuts();
    ensure_tray();
}

void MainWindow::startTracker() {
    if (work)
        return;
    
    // tracker dtor needs run first
    work = nullptr;

    libs = SelectedLibraries(ui.video_frame, current_tracker(), current_protocol(), current_filter());

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
        libs = SelectedLibraries();
        return;
    }
    
    work = std::make_shared<Work>(s, pose, libs, winId());
    
    reload_options();

    if (pTrackerDialog)
        pTrackerDialog->register_tracker(libs.pTracker.get());
    
    if (pFilterDialog)
        pFilterDialog->register_filter(libs.pFilter.get());
    
    if (pProtocolDialog)
        pProtocolDialog->register_protocol(libs.pProtocol.get());
    
    pose_update_timer.start(50);

    // NB check valid since SelectedLibraries ctor called
    // trackers take care of layout state updates
    const bool is_inertial = ui.video_frame->layout() == nullptr;
    updateButtonState(true, is_inertial);

    maybe_save();

    ui.btnStopTracker->setFocus();
}

void MainWindow::stopTracker() {
    if (!work)
        return;
    
    //ui.game_name->setText("Not connected");

    pose_update_timer.stop();
    ui.pose_display->rotateBy(0, 0, 0, 0, 0, 0);

    if (pTrackerDialog)
        pTrackerDialog->unregister_tracker();

    if (pProtocolDialog)
        pProtocolDialog->unregister_protocol();

    if (pFilterDialog)
        pFilterDialog->unregister_filter();

    maybe_save();

    work = nullptr;
    libs = SelectedLibraries();

    {
        double p[6] = {0,0,0, 0,0,0};
        display_pose(p, p);
    }
    updateButtonState(false, false);

    set_title();

    ui.btnStartTracker->setFocus();
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

    QString game_title;
    if (libs.pProtocol)
        game_title = libs.pProtocol->game_name();
    set_title(game_title);
}

void MainWindow::set_title(const QString& game_title_)
{
    QString game_title;
    if (game_title_ != "")
        game_title = " :: " + game_title_;
    QString current = group::ini_filename();
    setWindowTitle(opentrack_version + QStringLiteral(" :: ") + current + game_title);
}

void MainWindow::showHeadPose()
{
    double mapped[6], raw[6];

    work->tracker->get_raw_and_mapped_poses(mapped, raw);

    display_pose(mapped, raw);
}

template<typename t>
bool mk_dialog(mem<dylib> lib, mem<t>& orig)
{
    if (orig && orig->isVisible())
    {
        orig->show();
        orig->raise();
        return false;
    }

    if (lib && lib->Dialog)
    {
        auto dialog = mem<t>(reinterpret_cast<t*>(lib->Dialog()));
        dialog->setWindowFlags(Qt::Dialog);
        dialog->setFixedSize(dialog->size());

        orig = dialog;
        dialog->show();

        QObject::connect(dialog.get(), &plugin_api::detail::BaseDialog::closing, [&]() -> void { orig = nullptr; });

        return true;
    }

    return false;
}

void MainWindow::showTrackerSettings()
{
    if (mk_dialog(current_tracker(), pTrackerDialog) && libs.pTracker)
        pTrackerDialog->register_tracker(libs.pTracker.get());
}

void MainWindow::showProtocolSettings() {
    if (mk_dialog(current_protocol(), pProtocolDialog) && libs.pProtocol)
        pProtocolDialog->register_protocol(libs.pProtocol.get());
}

void MainWindow::showFilterSettings() {
    if (mk_dialog(current_filter(), pFilterDialog) && libs.pFilter)
        pFilterDialog->register_filter(libs.pFilter.get());
}

template<typename t, typename... Args>
bool mk_window(mem<t>* place, Args&&... params)
{
    if (*place && (*place)->isVisible())
    {
        (*place)->show();
        (*place)->raise();
        return false;
    }
    else
    {
        *place = std::make_shared<t>(std::forward<Args>(params)...);
        (*place)->setWindowFlags(Qt::Dialog);
        (*place)->show();
        return true;
    }
}

void MainWindow::show_options_dialog() {
    if (mk_window(&options_widget,
                  s,
                  [&]() -> void { register_shortcuts(); },
                  [&](bool flag) -> void { keys_paused = flag; }))
        connect(options_widget.get(), SIGNAL(reload()), this, SLOT(reload_options()));
}

void MainWindow::showCurveConfiguration() {
    mk_window(&mapping_widget, pose, s);
}

void MainWindow::exit() {
    QCoreApplication::exit(0);
}

void MainWindow::profileSelected(QString name)
{
    if (name == "" || is_refreshing_profiles)
        return;

    const auto old_name = group::ini_filename();
    const auto new_name = name;

    if (old_name != new_name)
    {
        save();

        {
            QSettings settings(OPENTRACK_ORG);
            settings.setValue (OPENTRACK_CONFIG_FILENAME_KEY, new_name);
        }

        set_title();
        load_settings();
    }
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
    QSettings settings(OPENTRACK_ORG);
    settings.setValue(OPENTRACK_CONFIG_FILENAME_KEY, profile);
}
