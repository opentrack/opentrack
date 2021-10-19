/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "main-window.hpp"
#include "logic/pipeline.hpp"
#include "options/options.hpp"
#include "new_file_dialog.h"
#include "migration/migration.hpp"
#include "compat/check-visible.hpp"
#include "compat/sleep.hpp"
#include "compat/macros.hpp"
#include "compat/library-path.hpp"
#include "compat/math.hpp"
#include "compat/sysexits.hpp"

#include <algorithm>
#include <utility>

#include <QMessageBox>
#include <QDesktopServices>
#include <QDir>

extern "C" const char* const opentrack_version;

using namespace options::globals;
using namespace options;

main_window::main_window() : State(OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH)
{
    ui.setupUi(this);

#if !defined _WIN32
    annoy_if_root();
#endif

    setWindowFlags(Qt::MSWindowsFixedSizeDialogHint | windowFlags());
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    init_profiles();
    init_buttons();
    init_dylibs();
    init_shortcuts();

    init_tray_menu();
    setVisible(!start_in_tray());
    ensure_tray();

    connect(&pose_update_timer, &QTimer::timeout,
            this, &main_window::show_pose, Qt::DirectConnection);
    connect(&det_timer, &QTimer::timeout,
            this, &main_window::maybe_start_profile_from_executable);
    det_timer.start(1000);
}

void main_window::init_shortcuts()
{
    register_shortcuts();

    // ctrl+q exits
    connect(&kbd_quit, &QShortcut::activated, this, [this]() { main_window::exit(EXIT_SUCCESS); }, Qt::DirectConnection);
    kbd_quit.setEnabled(true);
}

void main_window::init_dylibs()
{
    using dylib_ptr = Modules::dylib_ptr;
    using dylib_list = Modules::dylib_list;

    modules.filters().insert(modules.filters().begin(),
                             std::make_shared<dylib>("", dylib_type::Filter));

    for (dylib_ptr& x : modules.trackers())
        ui.iconcomboTrackerSource->addItem(x->icon, x->name, x->module_name);

    for (dylib_ptr& x : modules.protocols())
        ui.iconcomboProtocol->addItem(x->icon, x->name, x->module_name);

    for (dylib_ptr& x : modules.filters())
        ui.iconcomboFilter->addItem(x->icon, x->name, x->module_name);

    connect(ui.iconcomboTrackerSource, &QComboBox::currentTextChanged,
            this, [&](const QString&) { pTrackerDialog = nullptr; });

    connect(ui.iconcomboProtocol, &QComboBox::currentTextChanged,
            this, [&](const QString&) { pProtocolDialog = nullptr; });

    connect(ui.iconcomboFilter, &QComboBox::currentTextChanged,
            this, [&](const QString&) { pFilterDialog = nullptr; });

    connect(&m.tracker_dll, value_::value_changed<QString>(),
            this, &main_window::save_modules,
            Qt::DirectConnection);

    connect(&m.protocol_dll, value_::value_changed<QString>(),
            this, &main_window::save_modules,
            Qt::DirectConnection);

    connect(&m.filter_dll, value_::value_changed<QString>(),
            this, &main_window::save_modules,
            Qt::DirectConnection);

    {
        struct list {
            dylib_list& libs;
            QComboBox* input;
            value<QString>& place;
        };

        list types[] {
            { modules.trackers(), ui.iconcomboTrackerSource, m.tracker_dll },
            { modules.protocols(), ui.iconcomboProtocol, m.protocol_dll },
            { modules.filters(), ui.iconcomboFilter, m.filter_dll },
        };

        for (list& type : types)
        {
            list t = type;
            tie_setting(t.place, t.input,
                        [t](const QString& name) {
                            auto [ptr, idx] = module_by_name(name, t.libs);
                            return idx;
                        },
                        [t](int, const QVariant& userdata) {
                            auto [ptr, idx] = module_by_name(userdata.toString(), t.libs);
                            if (ptr)
                                return ptr->module_name;
                            else
                                return QString();
                        });
        }
    }
}

void main_window::init_profiles()
{
    refresh_profile_list();
    // implicitly created by `ini_directory()'
    if (ini_directory().isEmpty() || !QDir(ini_directory()).isReadable())
        die_on_profile_not_writable();

    set_profile(ini_filename());

    // profile menu
    profile_menu.addAction(tr("Create new empty config"), this, &main_window::create_empty_profile);
    profile_menu.addAction(tr("Create new copied config"), this, &main_window::create_copied_profile);
    profile_menu.addAction(tr("Open configuration directory"), this, &main_window::open_profile_directory);
    ui.profile_button->setMenu(&profile_menu);

    connect(&profile_list_timer, &QTimer::timeout, this, &main_window::refresh_profile_list);
    profile_list_timer.start(1000 * 5);

    connect(ui.iconcomboProfile, &QComboBox::currentTextChanged,
            this, [this](const QString& x) { main_window::set_profile(x); });
}

void main_window::init_tray_menu()
{
    tray_menu.clear();

    QString display_name(opentrack_version);
    if (display_name.startsWith("opentrack-"))
        display_name = tr("opentrack") + " " + display_name.mid(sizeof("opentrack-") - 1);
    if (display_name.endsWith("-DEBUG"))
        display_name.replace(display_name.size() - int(sizeof("DEBUG")), display_name.size(), tr(" (debug)"));

    menu_action_header.setEnabled(false);
    menu_action_header.setText(display_name);
    menu_action_header.setIcon(QIcon(":/images/opentrack.png"));
    tray_menu.addAction(&menu_action_header);

    menu_action_show.setIconVisibleInMenu(true);
    menu_action_show.setText(isHidden() ? tr("Show the Octopus") : tr("Hide the Octopus"));
    menu_action_show.setIcon(QIcon(":/images/opentrack.png"));
    QObject::connect(&menu_action_show, &QAction::triggered, this, [&] { toggle_restore_from_tray(QSystemTrayIcon::Trigger); });
    tray_menu.addAction(&menu_action_show);

    tray_menu.addSeparator();

    menu_action_tracker.setText(tr("Tracker settings"));
    menu_action_tracker.setIcon(QIcon(":/images/tools.png"));
    QObject::connect(&menu_action_tracker, &QAction::triggered, this, &main_window::show_tracker_settings);
    tray_menu.addAction(&menu_action_tracker);

    menu_action_filter.setText(tr("Filter settings"));
    menu_action_filter.setIcon(QIcon(":/images/filter-16.png"));
    QObject::connect(&menu_action_filter, &QAction::triggered, this, &main_window::show_filter_settings);
    tray_menu.addAction(&menu_action_filter);

    menu_action_proto.setText(tr("Protocol settings"));
    menu_action_proto.setIcon(QIcon(":/images/settings16.png"));
    QObject::connect(&menu_action_proto, &QAction::triggered, this, &main_window::show_proto_settings);
    tray_menu.addAction(&menu_action_proto);

    tray_menu.addSeparator();

    menu_action_mappings.setIcon(QIcon(":/images/curves.png"));
    menu_action_mappings.setText(tr("Mappings"));
    QObject::connect(&menu_action_mappings, &QAction::triggered, this, &main_window::show_mapping_window);
    tray_menu.addAction(&menu_action_mappings);

    menu_action_options.setIcon(QIcon(":/images/tools.png"));
    menu_action_options.setText(tr("Options"));
    QObject::connect(&menu_action_options, &QAction::triggered, this, &main_window::show_options_dialog);
    tray_menu.addAction(&menu_action_options);

    tray_menu.addSeparator();

    menu_action_exit.setText(tr("Exit"));
    QObject::connect(&menu_action_exit, &QAction::triggered, this, &main_window::exit);
    tray_menu.addAction(&menu_action_exit);

    connect(&s.tray_enabled, value_::value_changed<bool>(),
            this, &main_window::ensure_tray);
}

void main_window::init_buttons()
{
    update_button_state(false, false);
    connect(ui.btnEditCurves, &QPushButton::clicked, this, &main_window::show_mapping_window);
    connect(ui.btnShortcuts, &QPushButton::clicked, this, &main_window::show_options_dialog);
    connect(ui.btnShowEngineControls, &QPushButton::clicked, this, &main_window::show_tracker_settings);
    connect(ui.btnShowServerControls, &QPushButton::clicked, this, &main_window::show_proto_settings);
    connect(ui.btnShowFilterControls, &QPushButton::clicked, this, &main_window::show_filter_settings);
    connect(ui.btnStartTracker, &QPushButton::clicked, this, &main_window::start_tracker_);
    connect(ui.btnStopTracker, &QPushButton::clicked, this, &main_window::stop_tracker_);
}

void main_window::register_shortcuts()
{
    global_shortcuts.reload({
        { s.key_start_tracking1, [this](bool) { start_tracker(); }, true },
        { s.key_start_tracking2, [this](bool) { start_tracker(); }, true },

        { s.key_stop_tracking1, [this](bool) { stop_tracker(); }, true },
        { s.key_stop_tracking2, [this](bool) { stop_tracker(); }, true },

        { s.key_toggle_tracking1, [this](bool) { toggle_tracker(); }, true },
        { s.key_toggle_tracking2, [this](bool) { toggle_tracker(); }, true },

        { s.key_restart_tracking1, [this](bool) { restart_tracker(); }, true },
        { s.key_restart_tracking2, [this](bool) { restart_tracker(); }, true },
    });

    if (work)
        work->reload_shortcuts();
}

void main_window::die_on_profile_not_writable()
{
    stop_tracker_();

    static const QString pad(16, QChar(' '));

    QMessageBox::critical(this,
                          tr("The Octopus is sad"),
                          tr("Check permissions for your .ini directory:\n\n\"%1\"%2\n\nExiting now.").arg(ini_directory(), pad),
                          QMessageBox::Close, QMessageBox::NoButton);

    exit(EX_OSFILE);
}

bool main_window::profile_name_from_dialog(QString& ret)
{
    new_file_dialog dlg;
    dlg.exec();
    return dlg.is_ok(ret);
}

main_window::~main_window()
{
    // stupid ps3 eye has LED issues
    if (work && ui.video_frame->layout())
    {
        hide();
        stop_tracker_();
        close();

        portable::sleep(1000);
    }

    exit();
}

void main_window::save_modules()
{
    m.b->save();
}

void main_window::create_empty_profile()
{
    QString name;
    if (profile_name_from_dialog(name))
    {
        QFile(ini_combine(name)).open(QFile::ReadWrite);
        refresh_profile_list();

        if (profile_list.contains(name))
        {
            QSignalBlocker q(ui.iconcomboProfile);

            set_profile(name, false);
            mark_profile_as_not_needing_migration();
        }
    }
}

void main_window::create_copied_profile()
{
    const QString cur = ini_pathname();
    QString name;
    if (!cur.isEmpty() && profile_name_from_dialog(name))
    {
        const QString new_name = ini_combine(name);
        (void) QFile::remove(new_name);
        QFile::copy(cur, new_name);

        refresh_profile_list();

        if (profile_list.contains(name))
        {
            QSignalBlocker q(ui.iconcomboProfile);

            set_profile(name, false);
            mark_profile_as_not_needing_migration();
        }
    }

}

void main_window::open_profile_directory()
{
    QDesktopServices::openUrl("file:///" + QDir::toNativeSeparators(ini_directory()));
}

void main_window::refresh_profile_list()
{
    if (work)
        return;

    QStringList list = ini_list();
    QString current = ini_filename();

    if (list == profile_list)
        return;

    if (!list.contains(current))
        current = OPENTRACK_DEFAULT_PROFILE;

    profile_list = list;

    static const QIcon icon(":/images/settings16.png");

    QSignalBlocker l(ui.iconcomboProfile);

    ui.iconcomboProfile->clear();
    ui.iconcomboProfile->addItems(list);

    for (int i = 0; i < list.size(); i++)
        ui.iconcomboProfile->setItemIcon(i, icon);

    ui.iconcomboProfile->setCurrentText(current);
}



void main_window::update_button_state(bool running, bool inertialp)
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

void main_window::start_tracker_()
{
    if (work)
        return;

    work = std::make_shared<Work>(pose, ev, ui.video_frame, current_tracker(), current_protocol(), current_filter());

    if (!work->is_ok())
    {
        work = nullptr;
        return;
    }

    {
        double p[6] = {0,0,0, 0,0,0};
        show_pose_(p, p);
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

void main_window::stop_tracker_()
{
    if (!work)
        return;

    force_is_visible(true);
    with_tracker_teardown sentinel;

    pose_update_timer.stop();
    ui.pose_display->present(0,0,0, 0,0,0);

    if (pTrackerDialog)
        pTrackerDialog->unregister_tracker();

    if (pProtocolDialog)
        pProtocolDialog->unregister_protocol();

    if (pFilterDialog)
        pFilterDialog->unregister_filter();

    work = nullptr;

    {
        double p[6] {};
        show_pose_(p, p);
    }

    update_button_state(false, false);
    set_title();
    ui.btnStartTracker->setFocus();
}

void main_window::show_pose_(const double* mapped, const double* raw)
{
    ui.pose_display->present(mapped[Yaw], mapped[Pitch], -mapped[Roll],
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

void main_window::set_title(const QString& game_title)
{
    static const QString version{opentrack_version};
    static const QString sep { tr(" :: ") };
    static const QString pat1{ version + sep + "%1" + sep + "%2" };
    static const QString pat2{ version + sep + "%1" };

    const QString current = ini_filename();

    if (game_title.isEmpty())
        setWindowTitle(pat2.arg(current));
    else
        setWindowTitle(pat1.arg(current, game_title));
}

void main_window::show_pose()
{
    set_is_visible(*this);

    if (mapping_widget)
        mapping_widget->refresh_tab();

    if (!check_is_visible())
        return;

    double mapped[6], raw[6];

    work->pipeline_.raw_and_mapped_pose(mapped, raw);

    show_pose_(mapped, raw);
}

static void show_window(QWidget& d, bool fresh)
{
    if (fresh)
    {
        d.setWindowFlags(Qt::MSWindowsFixedSizeDialogHint | d.windowFlags());
        d.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        d.show();
        d.adjustSize();
#ifdef __APPLE__
        d.raise();
#endif
        d.activateWindow();
    }
    else
    {
        d.show();
#ifdef __APPLE__
        d.raise();
#endif
        d.activateWindow();
    }
}

template<typename t, typename F>
static bool mk_window_common(std::unique_ptr<t>& d, F&& fun)
{
    bool fresh = false;

    if (!d)
        d = fun(), fresh = !!d;

    if (d)
        show_window(*d, fresh);

    return fresh;
}

template<typename t, typename... Args>
static bool mk_window(std::unique_ptr<t>& place, Args&&... params)
{
    return mk_window_common(place, [&] {
        return std::make_unique<t>(params...);
    });
}

template<typename t>
static bool mk_dialog(std::unique_ptr<t>& place, const std::shared_ptr<dylib>& lib)
{
    using u = std::unique_ptr<t>;

    return mk_window_common(place, [&] {
        if (lib && lib->Dialog)
            return u{ (t*)lib->Dialog() };
        else
            return u{};
    });
}

void main_window::show_tracker_settings()
{
    if (mk_dialog(pTrackerDialog, current_tracker()) && work && work->libs.pTracker)
        pTrackerDialog->register_tracker(work->libs.pTracker.get());
    if (pTrackerDialog)
        QObject::connect(pTrackerDialog.get(), &ITrackerDialog::closing,
                         this, [this] { QTimer::singleShot(0, [this] { pTrackerDialog = nullptr; }); });
}

void main_window::show_proto_settings()
{
    if (mk_dialog(pProtocolDialog, current_protocol()) && work && work->libs.pProtocol)
        pProtocolDialog->register_protocol(work->libs.pProtocol.get());
    if (pProtocolDialog)
        QObject::connect(pProtocolDialog.get(), &IProtocolDialog::closing,
                         this, [this] { QTimer::singleShot(0, [this] { pProtocolDialog = nullptr; }); });
}

void main_window::show_filter_settings()
{
    if (mk_dialog(pFilterDialog, current_filter()) && work && work->libs.pFilter)
        pFilterDialog->register_filter(work->libs.pFilter.get());
    if (pFilterDialog)
        QObject::connect(pFilterDialog.get(), &IFilterDialog::closing,
                         this, [this] { QTimer::singleShot(0, [this] { pFilterDialog = nullptr; }); });
}

void main_window::show_options_dialog()
{
    if (mk_window(options_widget, [&](bool flag) { set_keys_enabled(!flag); }))
    {
        // XXX this should logically connect to a bundle
        // also doesn't work when switching profiles with options dialog open
        // move shortcuts to a separate bundle and add a migration -sh 20180218
        connect(options_widget.get(), &options_dialog::closing,
                this, &main_window::register_shortcuts);
    }
}

void main_window::show_mapping_window()
{
    mk_window(mapping_widget, pose);
}

void main_window::exit(int status)
{
    // don't use std::call_once here, leads to freeze in Microsoft's CRT
    // this function never needs reentrancy anyway

    // this is probably harmless, but better safe than sorry
    if (exiting_already)
        return;
    exiting_already = true;

    //qDebug() << "opentrack: exiting";

    if (tray)
        tray->hide();
    tray = nullptr;

    //close();
    QApplication::setQuitOnLastWindowClosed(true);
    QApplication::exit(status);
}

void main_window::set_profile(const QString& new_name_, bool migrate)
{
    QSignalBlocker b(ui.iconcomboProfile);

    QString new_name = new_name_;

    if (!profile_list.contains(new_name))
    {
        new_name = OPENTRACK_DEFAULT_PROFILE;
        if (!profile_list.contains(new_name))
            migrate = false;
    }

    const bool status = new_name != ini_filename();

    if (status)
        set_profile_in_registry(new_name);

    using bundler = options::detail::bundler;

    bundler::reload_no_notify();

    if (migrate)
        // migrations are for config layout changes and other user-visible
        // incompatibilities in future versions
        run_migrations();
    else
        mark_profile_as_not_needing_migration();

    bundler::notify();

    set_title();

    if (status)
        ui.iconcomboProfile->setCurrentText(new_name);
}

void main_window::ensure_tray()
{
    if (!tray_enabled())
    {
        if (!isVisible())
        {
            show();

#ifdef __APPLE__
            raise(); // for OSX
#endif
            activateWindow(); // for Windows
        }

        if (tray)
            tray->hide();
        tray = nullptr;

        QApplication::setQuitOnLastWindowClosed(true);
    }
    else
    {
        if (!tray)
        {
            tray = std::make_unique<QSystemTrayIcon>(this);
            tray->setIcon(QIcon(":/images/opentrack.png"));
            tray->setContextMenu(&tray_menu);
            tray->show();

            connect(tray.get(),
                    &QSystemTrayIcon::activated,
                    this,
                    &main_window::toggle_restore_from_tray);
        }

        QApplication::setQuitOnLastWindowClosed(false);
    }
}

void main_window::toggle_restore_from_tray(QSystemTrayIcon::ActivationReason e)
{
    switch (e)
    {
    // if we enable double click also then it causes
    // toggle back to the original state
    //case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::Trigger: // single click
        break;
    default:
        return;
    }

    ensure_tray();

    const bool is_minimized = isHidden() || !tray_enabled();

    menu_action_show.setText(!isHidden() ? tr("Show the Octopus") : tr("Hide the Octopus"));

    setVisible(is_minimized);

    if (is_minimized)
    {
#ifdef __APPLE__
        raise(); // for OSX
#endif
        activateWindow(); // for Windows
    }
    else
    {
        lower();
        clearFocus();
    }
}

bool main_window::maybe_hide_to_tray(QEvent* e)
{
    if (e->type() == QEvent::WindowStateChange &&
        (windowState() & Qt::WindowMinimized) &&
        tray_enabled())
    {
        e->accept();
        ensure_tray();
        hide();

        return true;
    }

    return false;
}

void main_window::closeEvent(QCloseEvent*)
{
    exit();
}

void main_window::maybe_start_profile_from_executable()
{
    if (!work)
    {
        QString profile;
        if (det.profile_to_start(profile) && profile_list.contains(profile))
        {
            set_profile(profile);
            start_tracker_();
        }
    }
    else
    {
        if (det.should_stop())
            stop_tracker_();
    }
}

void main_window::set_keys_enabled(bool flag)
{
    if (!flag)
    {
        if (work)
            work->sc.reload({});
        global_shortcuts.reload({});
    }
    else
        register_shortcuts();
}

void main_window::changeEvent(QEvent* e)
{
    if (!maybe_hide_to_tray(e))
        e->ignore();
}

bool main_window::event(QEvent* event)
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
        break;
        default:
        break;
        }
    }
    return QMainWindow::event(event);
}

bool main_window::tray_enabled()
{
    return s.tray_enabled && QSystemTrayIcon::isSystemTrayAvailable();
}

bool main_window::start_in_tray()
{
    return tray_enabled() && s.tray_start;
}

void main_window::set_profile_in_registry(const QString &profile)
{
    with_global_settings_object([&](QSettings& s) {
        s.setValue(OPENTRACK_PROFILE_FILENAME_KEY, profile);
        mark_global_ini_modified();
    });
}

void main_window::restart_tracker_()
{
    qDebug() << "restart tracker";

    stop_tracker_();
    start_tracker_();
}

void main_window::toggle_tracker_()
{
    qDebug() << "toggle tracker";

    if (work)
        stop_tracker_();
    else
        start_tracker_();
}

#if !defined _WIN32
#   include <unistd.h>
void main_window::annoy_if_root()
{
    if (geteuid() == 0)
    {
        struct lst {
            QString caption;
            QString msg;
            int sleep_ms;
        };

        const lst list[] = {
            {
                tr("Running as root is bad"),
                tr("Do not run as root. Set correct device node permissions."),
                1000,
            },
            {
                tr("Running as root is bad, seriously"),
                tr("Do not run as root. I'll keep whining at every startup."),
                3000,
            },
            {
                tr("Be annoyed, comprehensively."),
                tr("Don't run as root to remove these annoying messages."),
                0
            }
        };

        for (const auto& x : list)
        {
            QMessageBox::critical(this, x.caption, x.msg, QMessageBox::Ok);
            portable::sleep(x.sleep_ms);
        }
    }
}
#endif
