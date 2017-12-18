/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "main-window.hpp"
#include "logic/pipeline.hpp"
#include "options/options.hpp"
#include "opentrack-library-path.h"
#include "new_file_dialog.h"
#include "migration/migration.hpp"
#include "compat/check-visible.hpp"
#include "compat/sleep.hpp"

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

extern "C" const char* const opentrack_version;

#if !defined _WIN32 && !defined __APPLE__
#   include <unistd.h>
void MainWindow::annoy_if_root()
{
    if (geteuid() == 0)
    {
        for (unsigned k = 0; k < 2; k++)
        {
            portable::sleep(1 * 1000);
            QMessageBox::critical(this,
                                  tr("Running as root is bad"),
                                  tr("Do not run as root. Set correct device node permissions."),
                                  QMessageBox::Ok);
            portable::sleep(1 * 1000);
            QMessageBox::critical(this,
                                  tr("Running as root is bad, seriously"),
                                  tr("Do not run as root. I'll keep whining at every startup."),
                                  QMessageBox::Ok);
            portable::sleep(3 * 1000);
            QMessageBox::critical(this,
                                  tr("Running as root is really seriously bad"),
                                  tr("Do not run as root. Be annoyed, comprehensively."),
                                  QMessageBox::Ok);
        }
    }
}
#endif

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

#if !defined _WIN32 && !defined __APPLE__
    annoy_if_root();
#endif

    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setWindowFlags(Qt::MSWindowsFixedSizeDialogHint | windowFlags());
    }

    update_button_state(false, false);

    if (group::ini_directory().size() == 0)
    {
        die_on_config_not_writable();
        return;
    }

    if (!refresh_config_list())
        return;

    connect(ui.btnEditCurves, SIGNAL(clicked()), this, SLOT(show_mapping_window()));
    connect(ui.btnShortcuts, SIGNAL(clicked()), this, SLOT(show_options_dialog()));
    connect(ui.btnShowEngineControls, SIGNAL(clicked()), this, SLOT(show_tracker_settings()));
    connect(ui.btnShowServerControls, SIGNAL(clicked()), this, SLOT(show_proto_settings()));
    connect(ui.btnShowFilterControls, SIGNAL(clicked()), this, SLOT(show_filter_settings()));
    connect(ui.btnStartTracker, SIGNAL(clicked()), this, SLOT(start_tracker_()));
    connect(ui.btnStopTracker, SIGNAL(clicked()), this, SLOT(stop_tracker_()));
    connect(ui.iconcomboProfile, &QComboBox::currentTextChanged, this, [&](const QString& x) { set_profile(x); });

    // fill dylib comboboxen
    {
        modules.filters().push_front(std::make_shared<dylib>("", dylib::Filter));

        for (std::shared_ptr<dylib>& x : modules.trackers())
            ui.iconcomboTrackerSource->addItem(x->icon, x->name);

        for (std::shared_ptr<dylib>& x : modules.protocols())
            ui.iconcomboProtocol->addItem(x->icon, x->name);

        for (std::shared_ptr<dylib>& x : modules.filters())
            ui.iconcomboFilter->addItem(x->icon, x->name);
    }

    // timers
    connect(&config_list_timer, &QTimer::timeout, this, [this]() { refresh_config_list(); });
    connect(&pose_update_timer, SIGNAL(timeout()), this, SLOT(show_pose()), Qt::DirectConnection);
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
        connect(ui.iconcomboTrackerSource,
                &QComboBox::currentTextChanged,
                this,
                [&](const QString&) { if (pTrackerDialog) pTrackerDialog = nullptr; });

        connect(ui.iconcomboTrackerSource,
                &QComboBox::currentTextChanged,
                this,
                [&](const QString&) { if (pProtocolDialog) pProtocolDialog = nullptr; });

        connect(ui.iconcomboTrackerSource,
                &QComboBox::currentTextChanged,
                this,
                [&](const QString&) { if (pFilterDialog) pFilterDialog = nullptr; });

        connect(&m.tracker_dll, base_value::value_changed<QString>(), this, &MainWindow::save_modules, Qt::QueuedConnection);
        connect(&m.protocol_dll, base_value::value_changed<QString>(), this, &MainWindow::save_modules, Qt::QueuedConnection);
        connect(&m.filter_dll, base_value::value_changed<QString>(), this, &MainWindow::save_modules, Qt::QueuedConnection);

        tie_setting(m.tracker_dll, ui.iconcomboTrackerSource);
        tie_setting(m.protocol_dll, ui.iconcomboProtocol);
        tie_setting(m.filter_dll, ui.iconcomboFilter);
    }

    connect(this, &MainWindow::start_tracker,
            this, [&]() -> void { qDebug() << "start tracker"; start_tracker_(); },
            Qt::QueuedConnection);

    connect(this, &MainWindow::stop_tracker,
            this, [&]() -> void { qDebug() << "stop tracker"; stop_tracker_(); },
            Qt::QueuedConnection);

    connect(this, &MainWindow::toggle_tracker,
            this, [&]() -> void { qDebug() << "toggle tracker"; if (work) stop_tracker_(); else start_tracker_(); },
            Qt::QueuedConnection);

    connect(this, &MainWindow::restart_tracker,
            this, [&]() -> void { qDebug() << "restart tracker"; stop_tracker_(); start_tracker_(); },
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
    QObject::connect(&menu_action_tracker, &QAction::triggered, this, &MainWindow::show_tracker_settings);
    tray_menu.addAction(&menu_action_tracker);

    menu_action_filter.setText(tr("Filter settings"));
    menu_action_filter.setIcon(QIcon(":/images/filter-16.png"));
    QObject::connect(&menu_action_filter, &QAction::triggered, this, &MainWindow::show_filter_settings);
    tray_menu.addAction(&menu_action_filter);

    menu_action_proto.setText(tr("Protocol settings"));
    menu_action_proto.setIcon(QIcon(":/images/settings16.png"));
    QObject::connect(&menu_action_proto, &QAction::triggered, this, &MainWindow::show_proto_settings);
    tray_menu.addAction(&menu_action_proto);

    tray_menu.addSeparator();

    menu_action_mappings.setIcon(QIcon(":/images/curves.png"));
    menu_action_mappings.setText(tr("Mappings"));
    QObject::connect(&menu_action_mappings, &QAction::triggered, this, &MainWindow::show_mapping_window);
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
        t_key(s.key_start_tracking1, [&](bool) -> void { start_tracker(); }, true),
        t_key(s.key_start_tracking2, [&](bool) -> void { start_tracker(); }, true),

        t_key(s.key_stop_tracking1, [&](bool) -> void { stop_tracker(); }, true),
        t_key(s.key_stop_tracking2, [&](bool) -> void { stop_tracker(); }, true),

        t_key(s.key_toggle_tracking1, [&](bool) -> void { toggle_tracker(); }, true),
        t_key(s.key_toggle_tracking2, [&](bool) -> void { toggle_tracker(); }, true),

        t_key(s.key_restart_tracking1, [&](bool) -> void { restart_tracker(); }, true),
        t_key(s.key_restart_tracking2, [&](bool) -> void { restart_tracker(); }, true),
    };

    global_shortcuts.reload(keys);

    if (work)
        work->reload_shortcuts();
}

void MainWindow::die_on_config_not_writable()
{
    stop_tracker_();

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
    const bool writable =
        group::with_settings_object([&](QSettings& s) {
            return s.isWritable();
        });

    if (writable)
        return false;

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
    const bool just_stopping = bool(work);
    stop_tracker_();

    // stupid ps3 eye has LED issues
    if (just_stopping)
    {
        close();
        QEventLoop ev;
        ev.processEvents();
        portable::sleep(2000);
    }
}

void MainWindow::set_working_directory()
{
    QDir::setCurrent(OPENTRACK_BASE_PATH);
}

void MainWindow::save_modules()
{
    qDebug() << "save modules";
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

void MainWindow::update_button_state(bool running, bool inertialp)
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

void MainWindow::start_tracker_()
{
    if (work)
        return;

    {
        double p[6] = {0,0,0, 0,0,0};
        display_pose(p, p);
    }

    work = std::make_shared<Work>(pose, ev, ui.video_frame, current_tracker(), current_protocol(), current_filter());

    if (!work->is_ok())
    {
        work = nullptr;
        return;
    }

    if (pTrackerDialog)
        pTrackerDialog->register_tracker(work->libs.pTracker.get());

    if (pFilterDialog)
        pFilterDialog->register_filter(work->libs.pFilter.get());

    if (pProtocolDialog)
        pProtocolDialog->register_protocol(work->libs.pProtocol.get());

    pose_update_timer.start(50);

    // NB check valid since SelectedLibraries ctor called
    // trackers take care of layout state updates
    const bool is_inertial = ui.video_frame->layout() == nullptr;
    update_button_state(true, is_inertial);

    ui.btnStopTracker->setFocus();
}

void MainWindow::stop_tracker_()
{
    if (!work)
        return;

    with_tracker_teardown sentinel;

    pose_update_timer.stop();
    ui.pose_display->rotate_sync(0,0,0, 0,0,0);

    if (pTrackerDialog)
        pTrackerDialog->unregister_tracker();

    if (pProtocolDialog)
        pProtocolDialog->unregister_protocol();

    if (pFilterDialog)
        pFilterDialog->unregister_filter();

    work = nullptr;

    {
        double p[6] = {0,0,0, 0,0,0};
        display_pose(p, p);
    }

    update_button_state(false, false);
    set_title();
    ui.btnStartTracker->setFocus();
}

void MainWindow::display_pose(const double *mapped, const double *raw)
{
    ui.pose_display->rotate_async(mapped[Yaw], mapped[Pitch], -mapped[Roll],
                                  mapped[TX], mapped[TY], mapped[TZ]);

    QLCDNumber* raw_[] = {
        ui.raw_x, ui.raw_y, ui.raw_z,
        ui.raw_yaw, ui.raw_pitch, ui.raw_roll,
    };

    QLCDNumber* mapped_[] = {
        ui.pose_x, ui.pose_y, ui.pose_z,
        ui.pose_yaw, ui.pose_pitch, ui.pose_roll,
    };

    for (int k = 0; k < 6; k++)
    {
        raw_[k]->display(iround(raw[k]));
        mapped_[k]->display(iround(mapped[k]));
    }

    QString game_title;
    if (work && work->libs.pProtocol)
        game_title = work->libs.pProtocol->game_name();
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

void MainWindow::show_pose()
{
    set_is_visible(*this);

    if (mapping_widget)
        mapping_widget->refresh_tab();

    if (!check_is_visible())
        return;

    double mapped[6], raw[6];

    work->tracker->raw_and_mapped_pose(mapped, raw);

    display_pose(mapped, raw);
}

template<typename t, typename F>
bool MainWindow::mk_window_common(std::unique_ptr<t>& d, F&& ctor)
{
    if (d)
    {
        d->show();
        d->raise();

        return false;
    }
    else if ((d = std::unique_ptr<t>(ctor())))
    {
        QWidget& w = *d;

        w.setWindowFlags(Qt::MSWindowsFixedSizeDialogHint | w.windowFlags());

        w.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        w.show();
        w.adjustSize();
        w.setFixedSize(w.size());

        return true;
    }

    return false;
}

template<typename t, typename... Args>
inline bool MainWindow::mk_window(std::unique_ptr<t>& place, Args&&... params)
{
    return mk_window_common(place, [&]() { return new t(std::forward<Args>(params)...); });
}

template<typename t>
bool MainWindow::mk_dialog(std::shared_ptr<dylib> lib, std::unique_ptr<t>& d)
{
    const bool just_created = mk_window_common(d, [&]() -> t* {
        if (lib && lib->Dialog)
            return reinterpret_cast<t*>(lib->Dialog());
        return nullptr;
    });

    return just_created;
}

void MainWindow::show_tracker_settings()
{
    if (mk_dialog(current_tracker(), pTrackerDialog) && work && work->libs.pTracker)
        pTrackerDialog->register_tracker(work->libs.pTracker.get());
    if (pTrackerDialog)
        // must run bundle::reload(), don't remove next line
        QObject::connect(pTrackerDialog.get(), &ITrackerDialog::closing, this, [this]() { pTrackerDialog = nullptr; });
}

void MainWindow::show_proto_settings()
{
    if (mk_dialog(current_protocol(), pProtocolDialog) && work && work->libs.pProtocol)
        pProtocolDialog->register_protocol(work->libs.pProtocol.get());
    if (pProtocolDialog)
        // must run bundle::reload(), don't remove next line
        QObject::connect(pProtocolDialog.get(), &IProtocolDialog::closing, this, [this]() { pProtocolDialog = nullptr; });
}

void MainWindow::show_filter_settings()
{
    if (mk_dialog(current_filter(), pFilterDialog) && work && work->libs.pFilter)
        pFilterDialog->register_filter(work->libs.pFilter.get());
    if (pFilterDialog)
        // must run bundle::reload(), don't remove next line
        QObject::connect(pFilterDialog.get(), &IFilterDialog::closing, this, [this]() { pFilterDialog = nullptr; });
}

void MainWindow::show_options_dialog()
{
    if (mk_window(options_widget, [&](bool flag) -> void { set_keys_enabled(!flag); }))
    {
        connect(options_widget.get(), &OptionsDialog::closing, this, &MainWindow::register_shortcuts);
    }
}

void MainWindow::show_mapping_window()
{
    mk_window(mapping_widget, pose);
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

    set_title();
    options::detail::bundler::refresh_all_bundles();

    // migrations are for config layout changes and other user-visible
    // incompatibilities in future versions
    run_migrations();

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
            tray = std::make_unique<QSystemTrayIcon>(this);
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
        }
    ))
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
            start_tracker_();
        }
    }
    else
    {
        if (det.should_stop())
            stop_tracker_();
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

bool MainWindow::event(QEvent* event)
{
    using t = QEvent::Type;

    if (work)
    {
        switch (event->type())
        {
        case t::Hide:
        case t::WindowActivate:
        case t::WindowDeactivate:
        case t::WindowStateChange:
        case t::FocusIn:
            set_is_visible(*this, true);
            /*FALLTHROUGH*/
        default:
            break;
        }
    }
    return QMainWindow::event(event);
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
    group::with_global_settings_object([&](QSettings& s) {
        s.setValue(OPENTRACK_CONFIG_FILENAME_KEY, profile);
    });
}
