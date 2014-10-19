/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2011	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*********************************************************************************/
#include "facetracknoir.h"
#include "shortcuts.h"
#include "opentrack/tracker.h"
#include "curve-config.h"
#include <QFileDialog>

FaceTrackNoIR::FaceTrackNoIR() : QMainWindow(nullptr),
    b(bundle("opentrack-ui")),
    s(b),
    #if defined(_WIN32)
        keybindingWorker(NULL),
    #else
        keyCenter(this),
        keyToggle(this),
    #endif
    pose(std::vector<axis_opts*>{&s.a_x, &s.a_y, &s.a_z, &s.a_yaw, &s.a_pitch, &s.a_roll}),
    timUpdateHeadPose(this),
    kbd_quit(QKeySequence("Ctrl+Q"), this),
    no_feed_pixmap(":/uielements/no-feed.png"),
    module_list(dylib::enum_libraries())
{
    ui.setupUi(this);
    setFixedSize(size());
    ui.video_frame_label->setPixmap(no_feed_pixmap);
    updateButtonState(false, false);

    QDir::setCurrent(QCoreApplication::applicationDirPath());
    
    connect(ui.btnLoad, SIGNAL(clicked()), this, SLOT(open()));
    connect(ui.btnSave, SIGNAL(clicked()), this, SLOT(save()));
    connect(ui.btnSaveAs, SIGNAL(clicked()), this, SLOT(saveAs()));

    connect(ui.btnEditCurves, SIGNAL(clicked()), this, SLOT(showCurveConfiguration()));
    connect(ui.btnShortcuts, SIGNAL(clicked()), this, SLOT(showKeyboardShortcuts()));
    connect(ui.btnShowEngineControls, SIGNAL(clicked()), this, SLOT(showTrackerSettings()));
    connect(ui.btnShowServerControls, SIGNAL(clicked()), this, SLOT(showServerControls()));
    connect(ui.btnShowFilterControls, SIGNAL(clicked()), this, SLOT(showFilterControls()));

    filter_modules.push_back(nullptr);
    ui.iconcomboFilter->addItem(QIcon(), "");
    
    fill_combobox(dylib::Tracker, module_list, tracker_modules, ui.iconcomboTrackerSource);
    fill_combobox(dylib::Protocol, module_list, protocol_modules, ui.iconcomboProtocol);
    fill_combobox(dylib::Filter, module_list, filter_modules, ui.iconcomboFilter);
    
    fill_profile_combobox();

    tie_setting(s.tracker_dll, ui.iconcomboTrackerSource);
    tie_setting(s.protocol_dll, ui.iconcomboProtocol);
    tie_setting(s.filter_dll, ui.iconcomboFilter);

    connect(ui.btnStartTracker, SIGNAL(clicked()), this, SLOT(startTracker()));
    connect(ui.btnStopTracker, SIGNAL(clicked()), this, SLOT(stopTracker()));

    connect(ui.iconcomboProfile, SIGNAL(currentIndexChanged(int)), this, SLOT(profileSelected(int)));
    connect(&timUpdateHeadPose, SIGNAL(timeout()), this, SLOT(showHeadPose()));

#ifndef _WIN32
    connect(&keyCenter, SIGNAL(activated()), this, SLOT(shortcutRecentered()));
    connect(&keyToggle, SIGNAL(activated()), this, SLOT(shortcutToggled()));
#endif

    connect(&kbd_quit, SIGNAL(activated()), this, SLOT(exit()));
    kbd_quit.setEnabled(true);
}

FaceTrackNoIR::~FaceTrackNoIR()
{
    stopTracker();
    save();
}

void FaceTrackNoIR::fill_combobox(dylib::Type t, QList<ptr<dylib>> list, QList<ptr<dylib>>& out_list, QComboBox* cbx)
{
    for (auto x : list)
    {
        if (t && t == x->type)
        {
            cbx->addItem(x->icon, x->name);
            out_list.append(x);
        }
    }
}

QFrame* FaceTrackNoIR::video_frame() {
    return ui.video_frame;
}

void FaceTrackNoIR::open() {
     QFileDialog dialog(this);
     dialog.setFileMode(QFileDialog::ExistingFile);

     QString fileName = dialog.getOpenFileName(
                                this,
                                 tr("Open the settings file"),
                                 QCoreApplication::applicationDirPath() + "/settings/",
                                 tr("Settings file (*.ini);;All Files (*)"),
                                               NULL);

    if (! fileName.isEmpty() ) {
        {
            QSettings settings("opentrack");
            settings.setValue ("SettingsFile", QFileInfo(fileName).absoluteFilePath());
        }
        fill_profile_combobox();
        loadSettings();
    }
}

void FaceTrackNoIR::save_mappings() {
    pose.save_mappings();
}

#if defined(__unix) || defined(__linux) || defined(__APPLE__)
#   include <unistd.h>
#endif

void FaceTrackNoIR::save() {
    b->save();
    save_mappings();

#if defined(__unix) || defined(__linux)
    QSettings settings("opentrack");
    const QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
    QByteArray bytes = QFile::encodeName(currentFile);
    const char* filename_as_asciiz = bytes.constData();

    if (access(filename_as_asciiz, R_OK | W_OK))
    {
        QMessageBox::warning(this, "Something went wrong", "Check permissions and ownership for your .ini file!", QMessageBox::Ok, QMessageBox::NoButton);
    }
#endif
}

void FaceTrackNoIR::saveAs()
{
    QSettings settings("opentrack");
    QString oldFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save file"),
                                                    oldFile,
                                                    tr("Settings file (*.ini);;All Files (*)"));
    if (!fileName.isEmpty()) {

        QFileInfo newFileInfo ( fileName );
        if ((newFileInfo.exists()) && (oldFile != fileName)) {
            QFile newFileFile ( fileName );
            newFileFile.remove();
        }

        QFileInfo oldFileInfo ( oldFile );
        if (oldFileInfo.exists()) {
            QFile oldFileFile ( oldFile );
            oldFileFile.copy( fileName );
        }

        settings.setValue ("SettingsFile", fileName);
        save();
    }

    fill_profile_combobox();
}

void FaceTrackNoIR::load_mappings() {
    pose.load_mappings();
}

void FaceTrackNoIR::loadSettings() {
    b->reload();
    load_mappings();
}

extern "C" volatile const char* opentrack_version;

void FaceTrackNoIR::fill_profile_combobox()
{
     QSettings settings("opentrack");
     QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath()
                                            + "/settings/default.ini" ).toString();
     qDebug() << "Config file now" << currentFile;
     QFileInfo pathInfo ( currentFile );
     setWindowTitle(QString( const_cast<const char*>(opentrack_version) + QStringLiteral(" :: ")) + pathInfo.fileName());
     QDir settingsDir( pathInfo.dir() );
     auto iniFileList = settingsDir.entryList( QStringList { "*.ini" } , QDir::Files, QDir::Name );
     ui.iconcomboProfile->clear();
     for (auto x : iniFileList)
         ui.iconcomboProfile->addItem(QIcon(":/images/settings16.png"), x);
     ui.iconcomboProfile->setCurrentText(pathInfo.fileName());
}

void FaceTrackNoIR::updateButtonState(bool running, bool inertialp)
{
    bool not_running = !running;
    ui.iconcomboProfile->setEnabled ( not_running );
    ui.btnStartTracker->setEnabled ( not_running );
    ui.btnStopTracker->setEnabled ( running );
    ui.iconcomboProtocol->setEnabled ( not_running );
    ui.iconcomboFilter->setEnabled ( not_running );
    ui.iconcomboTrackerSource->setEnabled(not_running);
    ui.btnStartTracker->setEnabled(not_running);
    ui.btnStopTracker->setEnabled(running);
    ui.video_frame_label->setVisible(not_running || inertialp);
}

void FaceTrackNoIR::startTracker( ) {
    b->save();
    loadSettings();
    bindKeyboardShortcuts();
    
    // Tracker dtor needs run first
    tracker = nullptr;
    
    libs = SelectedLibraries(video_frame(), current_tracker(), current_protocol(), current_filter());

    if (!libs.correct)
    {
        QMessageBox::warning(this, "Library load error",
                             "One of libraries failed to load. Check installation.",
                             QMessageBox::Ok,
                             QMessageBox::NoButton);
        return;
    }

#if defined(_WIN32)
    keybindingWorker = new KeybindingWorker(*this, keyCenter, keyToggle);
    keybindingWorker->start();
#endif
    
    ui.video_frame->show();
    timUpdateHeadPose.start(50);
    tracker = std::make_shared<Tracker>(s, pose, libs);
    tracker->start();

    // NB check valid since SelectedLibraries ctor called
    // trackers take care of layout state updates
    const bool is_inertial = ui.video_frame->layout() == nullptr;
    updateButtonState(true, is_inertial);
}

void FaceTrackNoIR::stopTracker( ) {
    ui.game_name->setText("Not connected");
#if defined(_WIN32)
    if (keybindingWorker)
    {
        keybindingWorker->should_quit = true;
        keybindingWorker->wait();
        delete keybindingWorker;
        keybindingWorker = NULL;
    }
#endif
    timUpdateHeadPose.stop();
    ui.pose_display->rotateBy(0, 0, 0);
    
    if (pTrackerDialog)
    {
        pTrackerDialog->unregister_tracker();
        pTrackerDialog = nullptr;
    }
    
    if (pProtocolDialog)
    {
        pProtocolDialog->unregister_protocol();
        pProtocolDialog = nullptr;
    }
    
    if (pFilterDialog)
    {
        pFilterDialog->unregisterFilter();
        pFilterDialog = nullptr;
    }
    
    tracker = nullptr;
    libs = SelectedLibraries();
    updateButtonState(false, false);
}

void FaceTrackNoIR::showHeadPose()
{
    double mapped[6], raw[6];

    tracker->get_raw_and_mapped_poses(mapped, raw);

    ui.pose_display->rotateBy(mapped[Yaw], mapped[Roll], mapped[Pitch]);

    if (mapping_widget)
        mapping_widget->update();
    
    for (int i = 0; i < 6; i++)
    {
        mapped[i] = (int) mapped[i];
        raw[i] = (int) raw[i];
    }

    ui.lcdNumX->display(raw[TX]);
    ui.lcdNumY->display(raw[TY]);
    ui.lcdNumZ->display(raw[TZ]);
    ui.lcdNumRotX->display(raw[Yaw]);
    ui.lcdNumRotY->display(raw[Pitch]);
    ui.lcdNumRotZ->display(raw[Roll]);

    ui.lcdNumOutputPosX->display(mapped[TX]);
    ui.lcdNumOutputPosY->display(mapped[TY]);
    ui.lcdNumOutputPosZ->display(mapped[TZ]);
    ui.lcdNumOutputRotX->display(mapped[Yaw]);
    ui.lcdNumOutputRotY->display(mapped[Pitch]);
    ui.lcdNumOutputRotZ->display(mapped[Roll]);

    if (libs.pProtocol)
    {
        const QString name = libs.pProtocol->game_name();
        ui.game_name->setText(name);
    }
}

void FaceTrackNoIR::showTrackerSettings()
{
    ptr<dylib> lib = tracker_modules.value(ui.iconcomboTrackerSource->currentIndex(), nullptr);

    if (lib) {
        pTrackerDialog = ptr<ITrackerDialog>(reinterpret_cast<ITrackerDialog*>(lib->Dialog()));
        pTrackerDialog->setFixedSize(pTrackerDialog->size());
        pTrackerDialog->register_tracker(libs.pTracker.get());
        pTrackerDialog->show();
    }
}

void FaceTrackNoIR::showServerControls() {
    ptr<dylib> lib = protocol_modules.value(ui.iconcomboProtocol->currentIndex(), nullptr);

    if (lib) {
        pProtocolDialog = ptr<IProtocolDialog>(reinterpret_cast<IProtocolDialog*>(lib->Dialog()));
        pProtocolDialog->setFixedSize(pProtocolDialog->size());
        pProtocolDialog->show();
    }
}

void FaceTrackNoIR::showFilterControls() {
    ptr<dylib> lib = filter_modules.value(ui.iconcomboFilter->currentIndex(), nullptr);

    if (lib) {
        pFilterDialog = ptr<IFilterDialog>(reinterpret_cast<IFilterDialog*>(lib->Dialog()));
        pFilterDialog->setFixedSize(pFilterDialog->size());
        pFilterDialog->registerFilter(libs.pFilter.get());
        pFilterDialog->show();
    }
}
void FaceTrackNoIR::showKeyboardShortcuts() {
    shortcuts_widget = std::make_shared<KeyboardShortcutDialog>( this, this );
    shortcuts_widget->show();
    shortcuts_widget->raise();
}
void FaceTrackNoIR::showCurveConfiguration() {
    mapping_widget = std::make_shared<MapWidget>(pose, s, this);
    mapping_widget->show();
    mapping_widget->raise();
}

void FaceTrackNoIR::exit() {
    QCoreApplication::exit(0);
}

extern "C" volatile const char* opentrack_version;

void FaceTrackNoIR::profileSelected(int index)
{
    QSettings settings("opentrack");
    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
    QFileInfo pathInfo ( currentFile );
    settings.setValue ("SettingsFile", pathInfo.absolutePath() + "/" + ui.iconcomboProfile->itemText(index));
    loadSettings();
}

#if !defined(_WIN32)
void FaceTrackNoIR::bind_keyboard_shortcut(QxtGlobalShortcut& key, key_opts& k)
{
    key.setShortcut(QKeySequence::fromString(""));
    key.setDisabled();
    const int idx = k.key_index;
    if (idx > 0)
    {
        QString seq(global_key_sequences.value(idx, ""));
        if (!seq.isEmpty())
        {
            if (k.shift)
                seq = "Shift+" + seq;
            if (k.alt)
                seq = "Alt+" + seq;
            if (k.ctrl)
                seq = "Ctrl+" + seq;
            key.setShortcut(QKeySequence::fromString(seq, QKeySequence::PortableText));
            key.setEnabled();
        } else {
        key.setDisabled();
    }
    }
}
#else
static void bind_keyboard_shortcut(Key& key, key_opts& k)
{
    int idx = k.key_index;
    if (idx > 0)
    {
        key.keycode = 0;
        key.shift = key.alt = key.ctrl = 0;
        if (idx < global_windows_key_sequences.size())
            key.keycode = global_windows_key_sequences[idx];
        key.shift = k.shift;
        key.alt = k.alt;
        key.ctrl = k.ctrl;
    }
}
#endif

void FaceTrackNoIR::bindKeyboardShortcuts()
{
#if !defined(_WIN32)
    bind_keyboard_shortcut(keyCenter, s.center_key);
    bind_keyboard_shortcut(keyToggle, s.toggle_key);
#else
    bind_keyboard_shortcut(keyCenter, s.center_key);
    bind_keyboard_shortcut(keyToggle, s.toggle_key);
#endif
    if (tracker) /* running already */
    {
#if defined(_WIN32)
        if (keybindingWorker)
        {
            keybindingWorker->should_quit = true;
            keybindingWorker->wait();
            delete keybindingWorker;
            keybindingWorker = NULL;
        }
        keybindingWorker = new KeybindingWorker(*this, keyCenter, keyToggle);
        keybindingWorker->start();
#endif
    }
}

void FaceTrackNoIR::shortcutRecentered()
{
    qDebug() << "Center";
    if (s.dingp)
        QApplication::beep();
    if (tracker)
        tracker->center();
}

void FaceTrackNoIR::shortcutToggled()
{
    qDebug() << "Toggle";
    if (s.dingp)
        QApplication::beep();
    if (tracker)
        tracker->toggle_enabled();
}
