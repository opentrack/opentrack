/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "main-window.hpp"
#include "logic/tracker.h"
#include "options/options.hpp"
#include "opentrack-library-path.h"
#include "new_file_dialog.h"
#include "migration/migration.hpp"
#include <QFile>
#include <QFileDialog>
#include <QDesktopServices>
#include <QCoreApplication>
#include <QApplication>
#include <QIcon>
#include <QString>
#include <QChar>
#include <QSignalBlocker>

#ifdef _WIN32
#   include <windows.h>
#endif

extern "C" const char* opentrack_version;

MainWindow::MainWindow() :
    State(OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH),
    pose_update_timer(this),
    kbd_quit(QKeySequence("Ctrl+Q"), this),
    menu_action_header(&tray_menu),
    menu_action_show(&tray_menu),
    menu_action_exit(&tray_menu),
    menu_action_tracker(&tray_menu),
    menu_action_filter(&tray_menu),
    menu_action_proto(&tray_menu),
    menu_action_options(&tray_menu),
    menu_action_mappings(&tray_menu)
{
    ui.setupUi(this);
    setFixedSize(size());
    updateButtonState(false, false);

    if (group::ini_directory().size() == 0)
    {
        die_on_config_not_writable();
        return;
    }

    if (!refresh_config_list())
        return;

    connect(ui.btnEditCurves, SIGNAL(clicked()), this, SLOT(showCurveConfiguration()));
    connect(ui.btnShortcuts, SIGNAL(clicked()), this, SLOT(show_options_dialog()));
    connect(ui.btnShowEngineControls, SIGNAL(clicked()), this, SLOT(showTrackerSettings()));
    connect(ui.btnShowServerControls, SIGNAL(clicked()), this, SLOT(showProtocolSettings()));
    connect(ui.btnShowFilterControls, SIGNAL(clicked()), this, SLOT(showFilterSettings()));
    connect(ui.btnStartTracker, SIGNAL(clicked()), this, SLOT(startTracker()));
    connect(ui.btnStopTracker, SIGNAL(clicked()), this, SLOT(stopTracker()));
    connect(ui.iconcomboProfile, &QComboBox::currentTextChanged, this, [&](const QString& x) { set_profile(x); });

    // fill dylib comboboxen
    {
        modules.filters().push_front(std::make_shared<dylib>("", dylib::Filter));

        for (mem<dylib>& x : modules.trackers())
            ui.iconcomboTrackerSource->addItem(x->icon, x->name);

        for (mem<dylib>& x : modules.protocols())
            ui.iconcomboProtocol->addItem(x->icon, x->name);

        for (mem<dylib>& x : modules.filters())
            ui.iconcomboFilter->addItem(x->icon, x->name);
    }

    // timers
    connect(&config_list_timer, &QTimer::timeout, this, [this]() { refresh_config_list(); });
    connect(&pose_update_timer, SIGNAL(timeout()), this, SLOT(showHeadPose()));
    connect(&det_timer, SIGNAL(timeout()), this, SLOT(maybe_start_profile_from_executable()));

    // ctrl+q exits
    connect(&kbd_quit, SIGNAL(activated()), this, SLOT(exit()));

    // profile menu
    {
        profile_menu.addAction(tr("Create new empty config"), this, SLOT(make_empty_config()));
        profile_menu.addAction(tr("Create new copied config"), this, SLOT(make_copied_config()));
        profile_menu.addAction(tr("Open configuration directory"), this, SLOT(open_config_directory()));
        ui.profile_button->setMenu(&profile_menu);
    }

    if (!progn(
        const QString cur = group::ini_filename();
        if (is_config_listed(cur))
            return set_profile(cur);
        else
            return set_profile(OPENTRACK_DEFAULT_CONFIG);
    ))
        return;

    // only tie and connect main screen options after migrations are done
    // below is fine, set_profile() is called already

    // dylibs
    {
        connect(&m.tracker_dll,
                static_cast<void(base_value::*)(const QString&) const>(&base_value::valueChanged),
                this,
                [&](const QString&) { if (pTrackerDialog) pTrackerDialog = nullptr; save_modules(); });

        connect(&m.protocol_dll,
                static_cast<void(base_value::*)(const QString&) const>(&base_value::valueChanged),
                this,
                [&](const QString&) { if (pProtocolDialog) pProtocolDialog = nullptr; save_modules(); });

        connect(&m.filter_dll,
                static_cast<void(base_value::*)(const QString&) const>(&base_value::valueChanged),
                this,
                [&](const QString&) { if (pFilterDialog) pFilterDialog = nullptr; save_modules(); });
    }

    tie_setting(m.tracker_dll, ui.iconcomboTrackerSource);
    tie_setting(m.protocol_dll, ui.iconcomboProtocol);
    tie_setting(m.filter_dll, ui.iconcomboFilter);

    connect(this, &MainWindow::emit_start_tracker,
            this, [&]() -> void { qDebug() << "start tracker"; startTracker(); },
            Qt::QueuedConnection);

    connect(this, &MainWindow::emit_stop_tracker,
            this, [&]() -> void { qDebug() << "stop tracker"; stopTracker(); },
            Qt::QueuedConnection);

    connect(this, &MainWindow::emit_toggle_tracker,
            this, [&]() -> void { qDebug() << "toggle tracker"; if (work) stopTracker(); else startTracker(); },
            Qt::QueuedConnection);

    connect(this, &MainWindow::emit_restart_tracker,
            this, [&]() -> void { qDebug() << "restart tracker"; stopTracker(); startTracker(); },
            Qt::QueuedConnection);

    // tray
    {
        init_tray_menu();

        connect(&s.tray_enabled,
                static_cast<void (base_value::*)(bool) const>(&base_value::valueChanged),
                this,
                [&](bool) { ensure_tray(); });
        ensure_tray();
    }

    register_shortcuts();
    det_timer.start(1000);
    config_list_timer.start(1000 * 5);
    kbd_quit.setEnabled(true);
}

void MainWindow::init_tray_menu()
{
    tray_menu.clear();

    QString display_name(opentrack_version);
    if (display_name.startsWith("opentrack-"))
    {
        display_name = tr("opentrack") + " " + display_name.mid(sizeof("opentrack-") - 1);
    }
    if (display_name.endsWith("-DEBUG"))
        display_name.replace(display_name.size() - int(sizeof("DEBUG")), display_name.size(), tr(" (debug)"));

    menu_action_header.setEnabled(false);
    menu_action_header.setText(display_name);
    menu_action_header.setIcon(QIcon(":/images/facetracknoir.png"));
    tray_menu.addAction(&menu_action_header);

    menu_action_show.setIconVisibleInMenu(true);
    menu_action_show.setText(isHidden() ? tr("Show the Octopus") : tr("Hide the Octopus"));
    menu_action_show.setIcon(QIcon(":/images/facetracknoir.png"));
    QObject::connect(&menu_action_show, &QAction::triggered, this, [&]() { toggle_restore_from_tray(QSystemTrayIcon::Trigger); });
    tray_menu.addAction(&menu_action_show);

    tray_menu.addSeparator();

    menu_action_tracker.setText(tr("Tracker settings"));
    menu_action_tracker.setIcon(QIcon(":/images/tools.png"));
    QObject::connect(&menu_action_tracker, &QAction::triggered, this, &MainWindow::showTrackerSettings);
    tray_menu.addAction(&menu_action_tracker);

    menu_action_filter.setText(tr("Filter settings"));
    menu_action_filter.setIcon(QIcon(":/images/filter-16.png"));
    QObject::connect(&menu_action_filter, &QAction::triggered, this, &MainWindow::showFilterSettings);
    tray_menu.addAction(&menu_action_filter);

    menu_action_proto.setText(tr("Protocol settings"));
    menu_action_proto.setIcon(QIcon(":/images/settings16.png"));
    QObject::connect(&menu_action_proto, &QAction::triggered, this, &MainWindow::showProtocolSettings);
    tray_menu.addAction(&menu_action_proto);

    tray_menu.addSeparator();

    menu_action_mappings.setIcon(QIcon(":/images/curves.png"));
    menu_action_mappings.setText(tr("Mappings"));
    QObject::connect(&menu_action_mappings, &QAction::triggered, this, &MainWindow::showCurveConfiguration);
    tray_menu.addAction(&menu_action_mappings);

    menu_action_options.setIcon(QIcon(":/images/tools.png"));
    menu_action_options.setText(tr("Options"));
    QObject::connect(&menu_action_options, &QAction::triggered, this, &MainWindow::show_options_dialog);
    tray_menu.addAction(&menu_action_options);

    tray_menu.addSeparator();

    menu_action_exit.setText(tr("Exit"));
    QObject::connect(&menu_action_exit, &QAction::triggered, this, &MainWindow::exit);
    tray_menu.addAction(&menu_action_exit);
}

void MainWindow::register_shortcuts()
{
    using t_key = Shortcuts::t_key;
    using t_keys = Shortcuts::t_keys;

    t_keys keys
    {
        t_key(s.key_start_tracking, [&](bool) -> void { emit_start_tracker(); }, true),
        t_key(s.key_stop_tracking, [&](bool) -> void { emit_stop_tracker(); }, true),
        t_key(s.key_toggle_tracking, [&](bool) -> void { emit_toggle_tracker(); }, true),
        t_key(s.key_restart_tracking, [&](bool) -> void { emit_restart_tracker(); }, true),
    };

    global_shortcuts.reload(keys);

    if (work)
        work->reload_shortcuts();
}

void MainWindow::die_on_config_not_writable()
{
    stopTracker();

    static const QString pad(16, QChar(' '));

    QMessageBox::critical(this,
                          tr("The Octopus is sad"),
                          tr("Check permissions for your .ini directory:\n\n\"%1\"%2\n\nExiting now.").arg(group::ini_directory()).arg(pad),
                          QMessageBox::Close, QMessageBox::NoButton);

    // signals main() to short-circuit
    if (!isVisible())
        setEnabled(false);

    setVisible(false);

    // tray related
    qApp->setQuitOnLastWindowClosed(true);

    close();
}

bool MainWindow::maybe_die_on_config_not_writable(const QString& current, QStringList* ini_list_)
{
    const bool open = QFile(group::ini_combine(current)).open(QFile::ReadWrite);
    const QStringList ini_list = group::ini_list();

    if (!ini_list.contains(current) || !open)
    {
        die_on_config_not_writable();
        return true;
    }

    if (ini_list_ != nullptr)
        *ini_list_ = ini_list;

    return false;
}

bool MainWindow::get_new_config_name_from_dialog(QString& ret)
{
    new_file_dialog dlg;
    dlg.exec();
    return dlg.is_ok(ret);
}

MainWindow::~MainWindow()
{
    if (tray)
        tray->hide();
    stopTracker();
}

void MainWindow::set_working_directory()
{
    QDir::setCurrent(OPENTRACK_BASE_PATH);
}

void MainWindow::save_modules()
{
    m.b->save();
}

void MainWindow::make_empty_config()
{
    QString name;
    if (get_new_config_name_from_dialog(name))
    {
        QFile(group::ini_combine(name)).open(QFile::ReadWrite);

        if (!refresh_config_list())
            return;

        if (is_config_listed(name))
        {
            QSignalBlocker q(ui.iconcomboProfile);

            if (!set_profile(name))
                return;
            mark_config_as_not_needing_migration();
        }
    }
}

void MainWindow::make_copied_config()
{
    const QString cur = group::ini_pathname();
    QString name;
    if (cur != "" && get_new_config_name_from_dialog(name))
    {
        const QString new_name = group::ini_combine(name);
        (void) QFile::remove(new_name);
        QFile::copy(cur, new_name);

        if (!refresh_config_list())
            return;

        if (is_config_listed(name))
        {
            QSignalBlocker q(ui.iconcomboProfile);

            if (!set_profile(name))
                return;
            mark_config_as_not_needing_migration();
        }
    }

}

void MainWindow::open_config_directory()
{
    QDesktopServices::openUrl("file:///" + QDir::toNativeSeparators(group::ini_directory()));
}

bool MainWindow::refresh_config_list()
{
    if (work)
        return true;

    QStringList ini_list = group::ini_list();

    // check for sameness
    const bool exact_same = ini_list.size() > 0 && progn(
        if (ini_list.size() == ui.iconcomboProfile->count())
        {
            const int sz = ini_list.size();
            for (int i = 0; i < sz; i++)
            {
                if (ini_list[i] != ui.iconcomboProfile->itemText(i))
                    return false;
            }
            return true;
        }

        return false;
    );

    QString current = group::ini_filename();

    if (!ini_list.contains(current))
        current = OPENTRACK_DEFAULT_CONFIG_Q;

    if (maybe_die_on_config_not_writable(current, &ini_list))
        return false;

    if (exact_same)
        return true;

    const QIcon icon(":/images/settings16.png");

    QSignalBlocker l(ui.iconcomboProfile);

    ui.iconcomboProfile->clear();
    ui.iconcomboProfile->addItems(ini_list);

    for (int i = 0; i < ini_list.size(); i++)
        ui.iconcomboProfile->setItemIcon(i, icon);

    ui.iconcomboProfile->setCurrentText(current);

    return true;
}

void MainWindow::updateButtonState(bool running, bool inertialp)
{
    bool not_running = !running;
    ui.iconcomboProfile->setEnabled(not_running);
    ui.btnStartTracker->setEnabled(not_running);
    ui.btnStopTracker->setEnabled(running);
    ui.iconcomboProtocol->setEnabled(not_running);
    ui.iconcomboFilter->setEnabled(not_running);
    ui.iconcomboTrackerSource->setEnabled(not_running);
    ui.profile_button->setEnabled(not_running);
    ui.video_frame_label->setVisible(not_running || inertialp);
    if(not_running)
    {
        ui.video_frame_label->setPixmap(QPixmap(":/images/tracking-not-started.png"));
    }
    else {
        ui.video_frame_label->setPixmap(QPixmap(":/images/no-feed.png"));
    }
}

void MainWindow::startTracker()
{
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
        QMessageBox::warning(this, tr("Library load error"),
                             tr("One of libraries failed to load. Check installation."),
                             QMessageBox::Ok,
                             QMessageBox::NoButton);
        libs = SelectedLibraries();
        return;
    }

    save_modules();

    work = std::make_shared<Work>(pose, libs, winId());
    work->reload_shortcuts();

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

    ui.btnStopTracker->setFocus();
}

void MainWindow::stopTracker()
{
    if (!work)
        return;

    opts::set_teardown_flag(true); // XXX hack -sh 20160926

    pose_update_timer.stop();
    ui.pose_display->rotateBy(0, 0, 0, 0, 0, 0);

    if (pTrackerDialog)
        pTrackerDialog->unregister_tracker();

    if (pProtocolDialog)
        pProtocolDialog->unregister_protocol();

    if (pFilterDialog)
        pFilterDialog->unregister_filter();

    work = nullptr;
    libs = SelectedLibraries();

    {
        double p[6] = {0,0,0, 0,0,0};
        display_pose(p, p);
    }

    opts::set_teardown_flag(false); // XXX hack -sh 20160926

    updateButtonState(false, false);
    set_title();
    ui.btnStartTracker->setFocus();
}

void MainWindow::display_pose(const double *mapped, const double *raw)
{
    ui.pose_display->rotateBy(mapped[Yaw], mapped[Pitch], -mapped[Roll],
                              mapped[TX], mapped[TY], mapped[TZ]);

    if (mapping_widget)
        mapping_widget->update();

    double mapped_[6], raw_[6];

    for (int i = 0; i < 6; i++)
    {
        mapped_[i] = iround(mapped[i]);
        raw_[i] = iround(raw[i]);
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
        game_title = tr(" :: ") + game_title_;
    QString current = group::ini_filename();
    QString version(opentrack_version);
    version = tr("opentrack") + " " + version.mid(sizeof("opentrack-") - 1);
    setWindowTitle(version + tr(" :: ") + current + game_title);
}

void MainWindow::showHeadPose()
{
    double mapped[6], raw[6];

    work->tracker->get_raw_and_mapped_poses(mapped, raw);

    display_pose(mapped, raw);
}

template<typename t>
bool mk_dialog(mem<dylib> lib, ptr<t>& orig)
{
    if (orig && orig->isVisible())
    {
        orig->show();
        orig->raise();
        return false;
    }

    if (lib && lib->Dialog)
    {
        t* dialog = reinterpret_cast<t*>(lib->Dialog());
        dialog->setWindowFlags(Qt::Dialog);
        // HACK: prevent stderr whining by adding a few pixels
        dialog->setFixedSize(dialog->size() + QSize(4, 4));
        dialog->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        dialog->show();

        orig.reset(dialog);

        QObject::connect(dialog, &plugin_api::detail::BaseDialog::closing, [&]() -> void { orig = nullptr; });

        return true;
    }

    return false;
}

void MainWindow::showTrackerSettings()
{
    if (mk_dialog(current_tracker(), pTrackerDialog) && libs.pTracker)
        pTrackerDialog->register_tracker(libs.pTracker.get());
}

void MainWindow::showProtocolSettings()
{
    if (mk_dialog(current_protocol(), pProtocolDialog) && libs.pProtocol)
        pProtocolDialog->register_protocol(libs.pProtocol.get());
}

void MainWindow::showFilterSettings()
{
    if (mk_dialog(current_filter(), pFilterDialog) && libs.pFilter)
        pFilterDialog->register_filter(libs.pFilter.get());
}

template<typename t, typename... Args>
static bool mk_window(ptr<t>* place, Args&&... params)
{
    if (*place && (*place)->isVisible())
    {
        (*place)->show();
        (*place)->raise();
        return false;
    }
    else
    {
        *place = make_unique<t>(std::forward<Args>(params)...);
        (*place)->setWindowFlags(Qt::Dialog);
        // HACK: prevent stderr whining by adding a few pixels
        (*place)->setFixedSize((*place)->size() + QSize(4, 4));
        (*place)->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        (*place)->show();
        return true;
    }
}

void MainWindow::show_options_dialog()
{
    if (mk_window(&options_widget, [&](bool flag) -> void { set_keys_enabled(!flag); }))
    {
        connect(options_widget.get(), &OptionsDialog::closing, this, &MainWindow::register_shortcuts);
    }
}

void MainWindow::showCurveConfiguration()
{
    mk_window(&mapping_widget, pose);
}

void MainWindow::exit()
{
    QCoreApplication::exit(0);
}

bool MainWindow::set_profile(const QString& new_name_)
{
    if (!refresh_config_list())
        return false;

    QString new_name = new_name_;

    if (!is_config_listed(new_name))
        new_name = OPENTRACK_DEFAULT_CONFIG_Q;

    if (maybe_die_on_config_not_writable(new_name, nullptr))
        return false;

    ui.iconcomboProfile->setCurrentText(new_name);
    set_profile_in_registry(new_name);

    // migrations are for config layout changes and other user-visible
    // incompatibilities in future versions
    run_migrations();

    set_title();
    options::detail::bundler::refresh_all_bundles();

    return true;
}

void MainWindow::ensure_tray()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable())
        return;

    if (s.tray_enabled)
    {
        if (!tray)
        {
            tray = make_unique<QSystemTrayIcon>(this);
            tray->setIcon(QIcon(":/images/facetracknoir.png"));
            tray->setContextMenu(&tray_menu);
            tray->show();

            connect(tray.get(),
                    &QSystemTrayIcon::activated,
                    this,
                    &MainWindow::toggle_restore_from_tray);
        }
    }
    else
    {
        const bool is_hidden = isHidden() || !isVisible();

        if (is_hidden)
        {
            show();
            setVisible(true);

            raise(); // for OSX
            activateWindow(); // for Windows
        }

        if (tray)
            tray->hide();
        tray = nullptr;
    }
}

void MainWindow::toggle_restore_from_tray(QSystemTrayIcon::ActivationReason e)
{
    if (progn(
        switch (e)
        {
        // if we enable double click also then it causes
        // toggle back to the original state
        //case QSystemTrayIcon::DoubleClick:
        case QSystemTrayIcon::Trigger: // single click
            return false;
        default:
            return true;
    }))
    {
        return;
    }

    ensure_tray();

    const bool is_minimized = isHidden() || !is_tray_enabled();

    menu_action_show.setText(!isHidden() ? tr("Show the Octopus") : tr("Hide the Octopus"));

    setVisible(is_minimized);
    setHidden(!is_minimized);

    setWindowState(progn(
                       using ws = Qt::WindowStates;
                       if (is_minimized)
                           return ws(windowState() & (~Qt::WindowMinimized));
                       else
                           return ws(Qt::WindowNoState);
    ));

    if (is_minimized)
    {
        raise(); // for OSX
        activateWindow(); // for Windows
    }
    else
    {
        lower();
        clearFocus();
    }
}

bool MainWindow::maybe_hide_to_tray(QEvent* e)
{
    if (e->type() == QEvent::WindowStateChange &&
        (windowState() & Qt::WindowMinimized) &&
        is_tray_enabled())
    {
        e->accept();
        ensure_tray();
        hide();

        return true;
    }

    return false;
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

void MainWindow::set_keys_enabled(bool flag)
{
    if (!flag)
    {
        if (work)
            work->sc->reload({});
        global_shortcuts.reload({});
    }
    else
    {
        register_shortcuts();
    }
}

bool MainWindow::is_config_listed(const QString& name)
{
    const int sz = ui.iconcomboProfile->count();
    for (int i = 0; i < sz; i++)
        if (ui.iconcomboProfile->itemText(i) == name)
            return true;
    return false;
}

void MainWindow::changeEvent(QEvent* e)
{
    if (maybe_hide_to_tray(e))
        e->accept();
    else
    {
        QMainWindow::changeEvent(e);
    }
}

void MainWindow::closeEvent(QCloseEvent*)
{
    exit();
}

bool MainWindow::is_tray_enabled()
{
    return s.tray_enabled && QSystemTrayIcon::isSystemTrayAvailable();
}

bool MainWindow::start_in_tray()
{
    return s.tray_enabled && s.tray_start && QSystemTrayIcon::isSystemTrayAvailable();
}

void MainWindow::set_profile_in_registry(const QString &profile)
{
    QSettings settings(OPENTRACK_ORG);
    settings.setValue(OPENTRACK_CONFIG_FILENAME_KEY, profile);
}
