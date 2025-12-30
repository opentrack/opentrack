/* Copyright (c) 2013-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#undef NDEBUG
#include <cassert>

#include "main-window.hpp"
#include "logic/pipeline.hpp"
#include "options/options.hpp"
#include "new_file_dialog.h"
#include "migration/migration.hpp"
#include "compat/check-visible.hpp"
#include "compat/sleep.hpp"
#include "compat/macros.h"
#include "compat/library-path.hpp"
#include "compat/math.hpp"
#include "compat/sysexits.hpp"
#include "opentrack/defs.hpp"

#include <cstring>
#include <utility>

#include <QMessageBox>
#include <QDesktopServices>
#include <QApplication>

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>


#ifdef __APPLE__
void disable_appnap_start();
void disable_appnap_stop();
#endif

extern "C" const char* const opentrack_version;

using namespace options::globals;
using namespace options;

main_window::main_window() : State(OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH)
{
    ui.setupUi(this);

#if !defined _WIN32
    annoy_if_root();
#endif

    adjustSize();
    setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);
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
    connect(&*s.b, &options::bundle_::reloading, this, &main_window::register_shortcuts);
    connect(&*s.b, &options::bundle_::saving, this, &main_window::register_shortcuts);

    ui.btnStartTracker->setFocus();
#ifdef UI_NO_VIDEO_FEED
    fake_video_frame.resize(640, 480);
    fake_video_frame_parent.setVisible(false);
#endif
}

void main_window::init_shortcuts()
{
    assert(!global_shortcuts);
    global_shortcuts = std::make_unique<Shortcuts>();

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

#ifndef UI_NO_TRACKER_COMBOBOX
    for (dylib_ptr& x : modules.trackers())
        ui.iconcomboTrackerSource->addItem(x->icon, x->name, x->module_name);
#endif

    for (dylib_ptr& x : modules.protocols())
        ui.iconcomboProtocol->addItem(x->icon, x->name, x->module_name);

#ifndef UI_NO_FILTER_COMBOBOX
    for (dylib_ptr& x : modules.filters())
        ui.iconcomboFilter->addItem(x->icon, x->name, x->module_name);
#endif

#ifndef UI_NO_TRACKER_COMBOBOX
    connect(ui.iconcomboTrackerSource, &QComboBox::currentTextChanged,
            this, [this](const QString&) { pTrackerDialog = nullptr; if (options_widget) options_widget->tracker_module_changed(); });
#endif

    connect(ui.iconcomboProtocol, &QComboBox::currentTextChanged,
            this, [this](const QString&) { pProtocolDialog = nullptr; if (options_widget) options_widget->proto_module_changed(); });

#ifndef UI_NO_FILTER_COMBOBOX
    connect(ui.iconcomboFilter, &QComboBox::currentTextChanged,
            this, [this](const QString&) { pFilterDialog = nullptr; if (options_widget) options_widget->filter_module_changed(); });
#endif

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
#ifndef UI_NO_TRACKER_COMBOBOX
            { modules.trackers(), ui.iconcomboTrackerSource, m.tracker_dll },
#endif
            { modules.protocols(), ui.iconcomboProtocol, m.protocol_dll },
#ifndef UI_NO_FILTER_COMBOBOX
            { modules.filters(), ui.iconcomboFilter, m.filter_dll },
#endif
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
    copy_presets();
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
    QObject::connect(&menu_action_show, &QAction::triggered, this, [this] { toggle_restore_from_tray(QSystemTrayIcon::Trigger); });
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
    QObject::connect(&menu_action_options, &QAction::triggered, this, [this] { show_options_dialog(true); });
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
    connect(ui.btnShortcuts, &QPushButton::clicked, this, [this] { show_options_dialog(true); });
#ifndef UI_NO_TRACKER_SETTINGS_BUTTON
    connect(ui.btnShowEngineControls, &QPushButton::clicked, this, &main_window::show_tracker_settings);
#endif
    connect(ui.btnShowServerControls, &QPushButton::clicked, this, &main_window::show_proto_settings);
#ifndef UI_NO_FILTER_SETTINGS_BUTTON
    connect(ui.btnShowFilterControls, &QPushButton::clicked, this, &main_window::show_filter_settings);
#endif
    connect(ui.btnStartTracker, &QPushButton::clicked, this, &main_window::start_tracker_);
    connect(ui.btnStopTracker, &QPushButton::clicked, this, &main_window::stop_tracker_);
}

void main_window::register_shortcuts()
{
    global_shortcuts->reload({
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
#ifndef UI_NO_VIDEO_FEED
    if (work && ui.video_frame->layout())
#else
    if (work)
#endif
    {
        hide();
        stop_tracker_();
        close();

        portable::sleep(1000);
    }

#ifdef _WIN32
    global_shortcuts = {};
    KeybindingWorker::terminate();
#endif

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
        (void)create_profile_from_preset(name);
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
#ifndef UI_NO_FILTER_COMBOBOX
    ui.iconcomboFilter->setEnabled(not_running);
#endif
#ifndef UI_NO_TRACKER_COMBOBOX
    ui.iconcomboTrackerSource->setEnabled(not_running);
#endif
    ui.profile_button->setEnabled(not_running);
#ifndef UI_NO_VIDEO_FEED
    ui.video_frame_label->setVisible(not_running || inertialp);
    if(not_running)
    {
        ui.video_frame_label->setPixmap(QPixmap(":/images/tracking-not-started.png"));
    }
    else {
        ui.video_frame_label->setPixmap(QPixmap(":/images/no-feed.png"));
    }
#endif
}

void main_window::start_tracker_()
{
    if (work)
        return;

#ifdef __APPLE__
    disable_appnap_start();
#endif


#ifndef UI_NO_VIDEO_FEED
    auto* frame = ui.video_frame;
#else
    auto* frame = &fake_video_frame;
#endif
    work = std::make_shared<Work>(pose, frame, current_tracker(), current_protocol(), current_filter());

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
        pTrackerDialog->register_tracker(&*work->libs.pTracker);

    if (pFilterDialog && work->libs.pFilter)
        pFilterDialog->register_filter(&*work->libs.pFilter);

    if (pProtocolDialog)
        pProtocolDialog->register_protocol(&*work->libs.pProtocol);

    if (options_widget)
    {
        options_widget->register_tracker(&*work->libs.pTracker);
        options_widget->register_protocol(&*work->libs.pProtocol);
        if (work->libs.pFilter)
            options_widget->register_filter(&*work->libs.pFilter);
    }

    pose_update_timer.start(1000/30);

    // NB check valid since SelectedLibraries ctor called
    // trackers take care of layout state updates
    const bool is_inertial = frame->layout() == nullptr;
    update_button_state(true, is_inertial);

    ui.btnStopTracker->setFocus();
}

void main_window::stop_tracker_()
{
    if (!work)
        return;

#ifdef __APPLE__
    disable_appnap_stop();
#endif

    force_is_visible(true);
    with_tracker_teardown sentinel;

    pose_update_timer.stop();
    ui.pose_display->present(0,0,0, 0,0,0);

    if (options_widget)
    {
        // XXX TODO other module types
        options_widget->unregister_tracker();
    }

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

#ifndef UI_NO_RAW_DATA
    QLCDNumber* raw_[] = {
        ui.raw_x, ui.raw_y, ui.raw_z,
        ui.raw_yaw, ui.raw_pitch, ui.raw_roll,
    };
#endif
#ifndef UI_NO_GAME_DATA
    QLCDNumber* mapped_[] = {
        ui.pose_x, ui.pose_y, ui.pose_z,
        ui.pose_yaw, ui.pose_pitch, ui.pose_roll,
    };
#endif

#if !defined UI_NO_RAW_DATA || !defined UI_NO_GAME_DATA
    for (int k = 0; k < 6; k++)
#endif
    {
#ifndef UI_NO_RAW_DATA
        raw_[k]->display(iround(raw[k]));
#endif
#ifndef UI_NO_GAME_DATA
        mapped_[k]->display(iround(mapped[k]));
#endif
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

bool main_window::module_tabs_enabled() const
{
    return true;
#if 0
    enum module_tab_state { tabs_maybe = -1, tabs_disable, tabs_enable };

    static const auto force = progn(
        auto str = getenv("OPENTRACK_MODULE_TABS");
        if (!str || !*str)
            return tabs_maybe;
        constexpr const char* strings_for_false[] = {
            "0", "n", "f", "disable",
        };
        constexpr const char* strings_for_true[] = {
            "1", "y", "t", "enable",
        };
        for (const auto* x : strings_for_false)
            if (!strncasecmp(str, x, strlen(x)))
                return tabs_disable;
        for (const auto* x : strings_for_true)
            if (!strncasecmp(str, x, strlen(x)))
                return tabs_enable;
        qDebug() << "main-window: invalid boolean for OPENTRACK_MODULE_TABS:"
                 << QLatin1String{str};
        return tabs_maybe;
    );
    switch (force)
    {
    case tabs_disable: return false;
    case tabs_enable: return true;
    case tabs_maybe: (void)0;
    }
    auto* d = QApplication::desktop();
    if (!d)
        return false;
    // Windows 10: 40px,  Windows 11: 48px, KDE: 51px
    constexpr int taskbar_size = 51;
    constexpr int min_avail_height = 768 - taskbar_size;
    QRect rect = d->availableGeometry(this);
    return rect.height() >= min_avail_height;
#endif
}

static void show_window(QWidget& d, bool fresh)
{
    if (fresh)
    {
        d.setWindowFlag(Qt::MSWindowsFixedSizeDialogHint);
        d.adjustSize();
        d.setFixedSize(d.size());
        d.show();
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

template<typename t, typename... Args>
static bool mk_window(std::unique_ptr<t>& d, bool show, Args&&... params)
{
    bool fresh = false;

    if (!(d && d->isVisible()))
    {
        d = std::make_unique<t>(std::forward<Args>(params)...);
        fresh = !!d;
    }

    if (d)
    {
        if (show && !d->embeddable())
            show_window(*d, fresh);
    }

    return fresh;
}

template<typename Instance, typename Dialog>
static void show_module_settings(std::shared_ptr<Instance> instance,
                                 std::unique_ptr<Dialog>& dialog,
                                 const Modules::dylib_ptr& lib,
                                 std::unique_ptr<options_dialog>& options_widget,
                                 main_window* win,
                                 bool show,
                                 void(Dialog::*register_fun)(Instance*),
                                 void(options_dialog::*switch_tab_fun)())
{
    if (!lib || !lib->Dialog)
        return;

    bool fresh = !(dialog && dialog->isVisible());
    if (fresh)
        dialog = std::unique_ptr<Dialog>{(Dialog*)lib->Dialog()};
    bool embed = dialog->embeddable() && win->module_tabs_enabled();

    if (!embed)
    {
        if (fresh)
        {
            if (instance)
                ((*dialog).*register_fun)(&*instance);
        }
        if (show)
            show_window(*dialog, fresh);
    }
    else if (show)
    {
        bool fresh = !options_widget;
        win->show_options_dialog(false);
        ((*options_widget).*switch_tab_fun)();
        show_window(*options_widget, fresh);
    }
}

void main_window::show_tracker_settings_(bool show)
{
    show_module_settings(work ? work->libs.pTracker : nullptr, pTrackerDialog, current_tracker(),
                         options_widget, this, show,
                         &ITrackerDialog::register_tracker, &options_dialog::switch_to_tracker_tab);
}

void main_window::show_proto_settings_(bool show)
{
    show_module_settings(work ? work->libs.pProtocol : nullptr, pProtocolDialog, current_protocol(),
                         options_widget, this, show,
                         &IProtocolDialog::register_protocol, &options_dialog::switch_to_proto_tab);
}

void main_window::show_filter_settings_(bool show)
{
    show_module_settings(work ? work->libs.pFilter : nullptr, pFilterDialog, current_filter(),
                         options_widget, this, show,
                         &IFilterDialog::register_filter, &options_dialog::switch_to_filter_tab);
}

void main_window::show_options_dialog(bool show)
{
    if (options_widget && options_widget->isVisible())
    {
        if (show)
            show_window(*options_widget, false);
        return;
    }

    bool embed = module_tabs_enabled();

    if (embed)
    {
        show_tracker_settings_(false);
        show_proto_settings_(false);
        show_filter_settings_(false);
    }

    // make them into unique_ptr<BaseDialog>
    std::unique_ptr<ITrackerDialog> empty_ITD;
    std::unique_ptr<IProtocolDialog> empty_IPD;
    std::unique_ptr<IFilterDialog> empty_IFD;

    mk_window(options_widget, false,
              embed ? pTrackerDialog : empty_ITD,
              embed ? pProtocolDialog : empty_IPD,
              embed ? pFilterDialog : empty_IFD,
              [this](bool flag) { set_keys_enabled(!flag); });

    if (work)
    {
        if (work->libs.pTracker)
            options_widget->register_tracker(&*work->libs.pTracker);
        if (work->libs.pProtocol)
            options_widget->register_protocol(&*work->libs.pProtocol);
        if (work->libs.pFilter)
            options_widget->register_filter(&*work->libs.pFilter);
    }

    if (show)
        show_window(*options_widget, true);
}

void main_window::show_mapping_window()
{
    mk_window(mapping_widget, true, pose);
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
    bool do_refresh = false;

    if (!profile_list.contains(new_name_))
    {
        new_name = QStringLiteral(OPENTRACK_DEFAULT_PROFILE);
        if (!profile_list.contains(new_name))
        {
            migrate = false;
            do_refresh = true;
        }
    }

    if (create_profile_from_preset(new_name))
        migrate = true;

     if (do_refresh)
         refresh_profile_list();
     assert(profile_list.contains(new_name));

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

            connect(&*tray, &QSystemTrayIcon::activated,
                    this, &main_window::toggle_restore_from_tray);
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
        global_shortcuts->reload({});
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

void main_window::copy_presets()
{
    const QString preset_dir = library_path + "/presets/";
    const QDir dir{preset_dir};
    if (!dir.exists())
    {
        qDebug() << "no preset dir";
        return;
    }
    with_global_settings_object([&](QSettings& s) {
      const QString& key = QStringLiteral("last-preset-copy-time");
      const auto last_time = s.value(key, -1LL).toLongLong();
      for (const auto& file : dir.entryInfoList({ "*.ini" }, QDir::Files, QDir::Name))
      {
          if (file.fileName() == QStringLiteral("default.ini"))
              continue;
          if (last_time < file.lastModified().toSecsSinceEpoch())
          {
              qDebug() << "copy preset" << file.fileName();
              (void)QFile::copy(file.filePath(), ini_combine(file.fileName()));
          }
      }
      s.setValue(key, QDateTime::currentSecsSinceEpoch());
    });
}

bool main_window::create_profile_from_preset(const QString& name)
{
    const QString dest = ini_combine(name);

    if (QFile::exists(dest))
        return false;

    const QString& default_name = QStringLiteral(OPENTRACK_DEFAULT_PROFILE);
    const QString default_preset = (library_path + "/presets/%1").arg(default_name);

    if (QFile::exists(default_preset))
    {
        bool ret = QFile::copy(default_preset, dest);
        if (ret)
            qDebug() << "create profile" << name << "from default preset" << (ret ? "" : "FAILED!");
        return ret;
    }
    else
    {
        (void)QFile(ini_combine(name)).open(QFile::ReadWrite);
        return false;
    }
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
