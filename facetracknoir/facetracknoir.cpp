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
#include "tracker.h"
#include "curve-config.h"
#include <QFileDialog>

#if defined(__APPLE__)
#   define SONAME "dylib"
#elif defined(_WIN32)
#   define SONAME "dll"
#else
#   define SONAME "so"
#endif

#include <iostream>

#ifdef _MSC_VER
#   error "No support for MSVC anymore"
#else
#   define LIB_PREFIX "lib"
#endif

static bool get_metadata(DynamicLibrary* lib, QString& longName, QIcon& icon)
{
    Metadata* meta;
    if (!lib->Metadata || ((meta = lib->Metadata()), !meta))
        return false;
    meta->getFullName(&longName);
    meta->getIcon(&icon);
    delete meta;
    return true;
}

static void fill_combobox(const QString& filter, QList<DynamicLibrary*>& list, QComboBox& cbx)
{
    QDir settingsDir( QCoreApplication::applicationDirPath() );
    QStringList filenames = settingsDir.entryList( QStringList() << (LIB_PREFIX + filter + SONAME), QDir::Files, QDir::Name );
    for ( int i = 0; i < filenames.size(); i++) {
        QIcon icon;
        QString longName;
        QString str = filenames.at(i);
        DynamicLibrary* lib = new DynamicLibrary(str);
        qDebug() << "Loading" << str;
        std::cout.flush();
        if (!get_metadata(lib, longName, icon))
        {
            delete lib;
            continue;
        }
        list.push_back(lib);
        cbx.addItem(icon, longName);
    }
}

FaceTrackNoIR::FaceTrackNoIR(QWidget *parent) : QMainWindow(parent),
    tracker(nullptr),
#if defined(_WIN32)
    keybindingWorker(NULL),
#else
    keyCenter(this),
    keyToggle(this),
#endif
    b(bundle("opentrack-ui")),
    s(b),
    pose(std::vector<axis_opts*>{&s.a_x, &s.a_y, &s.a_z, &s.a_yaw, &s.a_pitch, &s.a_roll}),
    timUpdateHeadPose(this),
    pTrackerDialog(nullptr),
    pProtocolDialog(nullptr),
    pFilterDialog(nullptr),
    shortcuts_widget(nullptr),
    mapping_widget(nullptr),
    kbd_quit(QKeySequence("Ctrl+Q"), this),
    looping(0),
    video_frame_layout(new QVBoxLayout()),
    no_feed_pixmap(":/uielements/no-feed.png")
{
    ui.setupUi(this);
    setFixedSize(size());
    ui.video_frame_label->setPixmap(no_feed_pixmap);
    updateButtonState(false, false);

    QDir::setCurrent(QCoreApplication::applicationDirPath());

    fill_profile_cbx();

    connect(ui.btnLoad, SIGNAL(clicked()), this, SLOT(open()));
    connect(ui.btnSave, SIGNAL(clicked()), this, SLOT(save()));
    connect(ui.btnSaveAs, SIGNAL(clicked()), this, SLOT(saveAs()));

    connect(ui.btnEditCurves, SIGNAL(clicked()), this, SLOT(showCurveConfiguration()));
    connect(ui.btnShortcuts, SIGNAL(clicked()), this, SLOT(showKeyboardShortcuts()));
    connect(ui.btnShowEngineControls, SIGNAL(clicked()), this, SLOT(showTrackerSettings()));
    connect(ui.btnShowServerControls, SIGNAL(clicked()), this, SLOT(showServerControls()));
    connect(ui.btnShowFilterControls, SIGNAL(clicked()), this, SLOT(showFilterControls()));

    dlopen_filters.push_back((DynamicLibrary*) NULL);
    ui.iconcomboFilter->addItem(QIcon(), "");

    fill_combobox("opentrack-proto-*.", dlopen_protocols, *ui.iconcomboProtocol);
    fill_combobox("opentrack-tracker-*.", dlopen_trackers, *ui.iconcomboTrackerSource);
    fill_combobox("opentrack-filter-*.", dlopen_filters, *ui.iconcomboFilter);

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

FaceTrackNoIR::~FaceTrackNoIR() {

    stopTracker();
    save();
    if (Libraries)
        delete Libraries;
    delete video_frame_layout;
}

QFrame* FaceTrackNoIR::get_video_widget() {
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
        fill_profile_cbx();
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
    looping++;
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

    looping--;
    fill_profile_cbx();
}

void FaceTrackNoIR::load_mappings() {
    pose.load_mappings();
}

void FaceTrackNoIR::loadSettings() {
    b->reload();
    load_mappings();
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

    if (Libraries)
        delete Libraries;
    Libraries = new SelectedLibraries(this);

    if (!Libraries->correct)
    {
        QMessageBox::warning(this, "Something went wrong", "Tracking can't be initialized, probably protocol prerequisites missing", QMessageBox::Ok, QMessageBox::NoButton);
        stopTracker();
        return;
    }

#if defined(_WIN32)
    keybindingWorker = new KeybindingWorker(*this, keyCenter, keyToggle);
    keybindingWorker->start();
#endif

    if (tracker) {
        delete tracker;
    }

    tracker = new Tracker(s, pose);

    if (pTrackerDialog && Libraries->pTracker) {
        pTrackerDialog->registerTracker( Libraries->pTracker );
    }

    if (pFilterDialog && Libraries->pFilter)
        pFilterDialog->registerFilter(Libraries->pFilter);

    tracker->start();

    ui.video_frame->show();

    timUpdateHeadPose.start(50);

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

    if (pTrackerDialog) {
        pTrackerDialog->unRegisterTracker();
        delete pTrackerDialog;
        pTrackerDialog = nullptr;
    }
    if (pProtocolDialog) {
        pProtocolDialog->unRegisterProtocol();
        delete pProtocolDialog;
        pProtocolDialog = nullptr;
    }
    if (pFilterDialog)
    {
        pFilterDialog->unregisterFilter();
        delete pFilterDialog;
        pFilterDialog = nullptr;
    }

    if ( tracker ) {
        delete tracker;
        tracker = 0;
        if (Libraries) {
            delete Libraries;
            Libraries = NULL;
        }
    }
    updateButtonState(false, false);
}

void FaceTrackNoIR::showHeadPose()
{
    double mapped[6], raw[6];

    tracker->get_raw_and_mapped_poses(mapped, raw);

    ui.pose_display->rotateBy(mapped[Yaw], mapped[Roll], mapped[Pitch]);

    if (mapping_widget)
        mapping_widget->update();

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

    if (Libraries->pProtocol)
    {
        const QString name = Libraries->pProtocol->getGameName();
        ui.game_name->setText(name);
    }
}

void FaceTrackNoIR::showTrackerSettings()
{
    if (pTrackerDialog) {
        delete pTrackerDialog;
        pTrackerDialog = NULL;
    }

    DynamicLibrary* lib = dlopen_trackers.value(ui.iconcomboTrackerSource->currentIndex(), (DynamicLibrary*) NULL);

    if (lib) {
        pTrackerDialog = (ITrackerDialog*) lib->Dialog();
        if (pTrackerDialog) {
            auto foo = dynamic_cast<QWidget*>(pTrackerDialog);
            foo->setFixedSize(foo->size());
            if (Libraries && Libraries->pTracker)
                pTrackerDialog->registerTracker(Libraries->pTracker);
            dynamic_cast<QWidget*>(pTrackerDialog)->show();
        }
    }
}

void FaceTrackNoIR::showServerControls() {
    if (pProtocolDialog) {
        delete pProtocolDialog;
        pProtocolDialog = NULL;
    }

    DynamicLibrary* lib = dlopen_protocols.value(ui.iconcomboProtocol->currentIndex(), (DynamicLibrary*) NULL);

    if (lib && lib->Dialog) {
        pProtocolDialog = (IProtocolDialog*) lib->Dialog();
        if (pProtocolDialog) {
            auto foo = dynamic_cast<QWidget*>(pProtocolDialog);
            foo->setFixedSize(foo->size());
            dynamic_cast<QWidget*>(pProtocolDialog)->show();
        }
    }
}

void FaceTrackNoIR::showFilterControls() {
    if (pFilterDialog) {
        delete pFilterDialog;
        pFilterDialog = NULL;
    }

    DynamicLibrary* lib = dlopen_filters.value(ui.iconcomboFilter->currentIndex(), (DynamicLibrary*) NULL);

    if (lib && lib->Dialog) {
        pFilterDialog = (IFilterDialog*) lib->Dialog();
        if (pFilterDialog) {
            auto foo = dynamic_cast<QWidget*>(pFilterDialog);
            foo->setFixedSize(foo->size());
            if (Libraries && Libraries->pFilter)
                pFilterDialog->registerFilter(Libraries->pFilter);
            dynamic_cast<QWidget*>(pFilterDialog)->show();
        }
    }
}
void FaceTrackNoIR::showKeyboardShortcuts() {

    if (!shortcuts_widget)
    {
        shortcuts_widget = new KeyboardShortcutDialog( this, this );
    }

    shortcuts_widget->show();
    shortcuts_widget->raise();
}
void FaceTrackNoIR::showCurveConfiguration() {
    if (mapping_widget)
        delete mapping_widget;
    
    mapping_widget = new MapWidget(pose, s, this);

    mapping_widget->show();
    mapping_widget->raise();
}

void FaceTrackNoIR::exit() {
    QCoreApplication::exit(0);
}

extern "C" volatile const char* opentrack_version;

void FaceTrackNoIR::fill_profile_cbx()
{
    if (looping)
        return;
    looping++;
    QSettings settings("opentrack");
    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
    qDebug() << "Config file now" << currentFile;
    QFileInfo pathInfo ( currentFile );
    setWindowTitle(QString( const_cast<const char*>(opentrack_version) + QStringLiteral(" :: ")) + pathInfo.fileName());
    QDir settingsDir( pathInfo.dir() );
    QStringList filters;
    filters << "*.ini";
    auto iniFileList = settingsDir.entryList( filters, QDir::Files, QDir::Name );
    ui.iconcomboProfile->clear();
    for ( int i = 0; i < iniFileList.size(); i++)
        ui.iconcomboProfile->addItem(QIcon(":/images/settings16.png"), iniFileList.at(i));
    ui.iconcomboProfile->setCurrentText(pathInfo.fileName());
    looping--;
}

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
