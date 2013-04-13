/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
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

#ifndef FaceTrackNoIR_H
#define FaceTrackNoIR_H

#undef FTNOIR_PROTOCOL_BASE_LIB
#undef FTNOIR_TRACKER_BASE_LIB
#undef FTNOIR_FILTER_BASE_LIB
#define FTNOIR_PROTOCOL_BASE_EXPORT Q_DECL_IMPORT
#define FTNOIR_TRACKER_BASE_EXPORT Q_DECL_IMPORT
#define FTNOIR_FILTER_BASE_EXPORT Q_DECL_IMPORT

#include <QtGui/QMainWindow>
#include <QApplication>
#include <QFileDialog>
#include <QListView>
#include <QPainter>
#include <QWidget>
#include <QDialog>
#include <QUrl>
#include <QList>
#include <QKeySequence>
#include <QtGui>
#include <QString>
#if !defined(_WIN32) && !defined(__WIN32)
#	include <qxtglobalshortcut.h>
#else
#	include <windows.h>
#endif
#include <QThread>
#include <QDebug>
#include <QElapsedTimer>

#include "ftnoir_posewidget/glwidget.h"

#include "ui_facetracknoir.h"
#include "ui_ftnoir_keyboardshortcuts.h"
#include "ui_ftnoir_preferences.h"
#include "ui_ftnoir_curves.h"

#include "ftnoir_protocol_base/ftnoir_protocol_base.h"
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ftnoir_filter_base/ftnoir_filter_base.h"

#include "global-settings.h"

class Tracker;				// pre-define class to avoid circular includes
class FaceTrackNoIR;

class KeybindingWorker;

#if defined(__WIN32) || defined(_WIN32)
extern QList<int> global_windows_key_sequences;
#undef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
struct Key {
    BYTE keycode;
    bool shift;
    bool ctrl;
    bool alt;
    bool ever_pressed;
    QElapsedTimer timer;
public:
    Key() : keycode(0), shift(false), ctrl(false), alt(false), ever_pressed(false)
    {
    }
};
#else
typedef unsigned char BYTE;
struct Key { int foo; };
#endif

class FaceTrackNoIR : public QMainWindow, IDynamicLibraryProvider
{
	Q_OBJECT

public:
	FaceTrackNoIR(QWidget *parent = 0, Qt::WFlags flags = 0);
	~FaceTrackNoIR();

	void updateSettings();						// Update the settings (let Tracker read INI-file).

    QFrame *get_video_widget();					// Get a pointer to the video-widget, to use in the DLL
    Tracker *tracker;
    void bindKeyboardShortcuts();
    DynamicLibrary* current_tracker1() {
        return dlopen_trackers.value(ui.iconcomboTrackerSource->currentIndex(), (DynamicLibrary*) NULL);
    }
    DynamicLibrary* current_tracker2() {
        return dlopen_trackers.value(ui.cbxSecondTrackerSource->currentIndex() - 1, (DynamicLibrary*) NULL);
    }
    DynamicLibrary* current_protocol() {
        return dlopen_protocols.value(ui.iconcomboProtocol->currentIndex(), (DynamicLibrary*) NULL);
    }
    DynamicLibrary* current_filter() {
        return dlopen_filters.value(ui.iconcomboFilter->currentIndex(), (DynamicLibrary*) NULL);
    }
#if defined(_WIN32) || defined(__WIN32)
    Key keyCenter, keyZero, keyStartStop, keyInhibit;
    KeybindingWorker* keybindingWorker;
#else 
    QxtGlobalShortcut* keyCenter;
    QxtGlobalShortcut* keyZero;
    QxtGlobalShortcut* keyStartStop;
    QxtGlobalShortcut* keyInhibit;
#endif
public slots:
        void shortcutRecentered();
        void shortcutZero();
        void shortcutStartStop();
        void shortcutInhibit();

private:
	Ui::FaceTrackNoIRClass ui;
	QTimer *timMinimizeFTN;						// Timer to Auto-minimize
	QTimer *timUpdateHeadPose;					// Timer to display headpose
	QStringList iniFileList;					// List of INI-files, that are present in the Settings folder

    ITrackerDialog* pTrackerDialog;			// Pointer to Tracker dialog instance (in DLL)
    ITrackerDialog* pSecondTrackerDialog;		// Pointer to the second Tracker dialog instance (in DLL)
    IProtocolDialog* pProtocolDialog;			// Pointer to Protocol dialog instance (in DLL)
    IFilterDialog* pFilterDialog;				// Pointer to Filter dialog instance (in DLL)

	/** Widget variables **/
	QVBoxLayout *l;
	QWidget *_preferences;
	QWidget *_keyboard_shortcuts;
	QWidget *_curve_config;
	GLWidget *_pose_display;

	/** QT objects **/
	QDialog aboutDialog;	
	QDesktopWidget desktop;

    QAction *minimizeAction;
    QAction *restoreAction;
    QAction *quitAction;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

	void createIconGroupBox();
//	void createMessageGroupBox();
	void createActions();
	void createTrayIcon();

	/** helper **/
	bool cameraDetected;
	bool settingsDirty;

	void GetCameraNameDX();
	void loadSettings();
	void setupFaceTrackNoIR();

    QList<DynamicLibrary*> dlopen_filters;
    QList<DynamicLibrary*> dlopen_trackers;
    QList<DynamicLibrary*> dlopen_protocols;

    bool looping;

	private slots:
		//file menu
		void open();
		void save();
		void saveAs();
		void exit();

		//about menu
		void openurl_support();
		void openurl_donation();
		void about();

//		void setIcon(int index);
		void iconActivated(QSystemTrayIcon::ActivationReason reason);
		void profileSelected(int index);
		void protocolSelected(int index);
		void filterSelected(int index);
		void trackingSourceSelected(int index);

		void showVideoWidget();
		void showHeadPoseWidget();
		void showTrackerSettings();
		void showSecondTrackerSettings();

		void showServerControls();
		void showFilterControls();
		void showPreferences();
		void showKeyboardShortcuts();
		void showCurveConfiguration();

        void setInvertAxis( Axis axis, int invert );

		void showHeadPose();

        void startTracker();
		void stopTracker();

};

// Widget that has controls for FaceTrackNoIR Preferences.
class PreferencesDialog: public QWidget, public Ui::UICPreferencesDialog
{
    Q_OBJECT
public:

	explicit PreferencesDialog( FaceTrackNoIR *ftnoir, QWidget *parent=0, Qt::WindowFlags f=0 );
    virtual ~PreferencesDialog();
	void showEvent ( QShowEvent * event );

private:
	Ui::UICPreferencesDialog ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;
	FaceTrackNoIR *mainApp;

private slots:
	void doOK();
	void doCancel();
	void keyChanged( int index ) { settingsDirty = true; };
};

// Widget that has controls for Keyboard shortcuts.
class KeyboardShortcutDialog: public QWidget, public Ui::UICKeyboardShortcutDialog
{
    Q_OBJECT
public:

	explicit KeyboardShortcutDialog( FaceTrackNoIR *ftnoir, QWidget *parent=0, Qt::WindowFlags f=0 );
    virtual ~KeyboardShortcutDialog();
	void showEvent ( QShowEvent * event );

private:
	Ui::UICKeyboardShortcutDialog ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;
	FaceTrackNoIR *mainApp;

private slots:
	void doOK();
	void doCancel();
	void keyChanged( int index ) { settingsDirty = true; };
	void keyChanged( bool index ) { settingsDirty = true; };
};

// Widget that has controls for Keyboard shortcuts.
class CurveConfigurationDialog: public QWidget, public Ui::UICCurveConfigurationDialog
{
    Q_OBJECT
public:

	explicit CurveConfigurationDialog( FaceTrackNoIR *ftnoir, QWidget *parent=0, Qt::WindowFlags f=0 );
    virtual ~CurveConfigurationDialog();
	void showEvent ( QShowEvent * event );

private:
	Ui::UICCurveConfigurationDialog ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;
	FaceTrackNoIR *mainApp;

private slots:
	void doOK();
	void doCancel();
    void curveChanged( bool change ) { settingsDirty = true; }
    void curveChanged( int change ) { settingsDirty = true; }
};

#endif // FaceTrackNoIR_H

extern QList<QString> global_key_sequences;
#if defined(__WIN32) || defined(_WIN32)
class KeybindingWorkerDummy {
private:
    LPDIRECTINPUT8 din;
    LPDIRECTINPUTDEVICE8 dinkeyboard;
    Key kCenter, kInhibit, kStartStop, kZero;
    FaceTrackNoIR& window;
public:
    volatile bool should_quit;
    ~KeybindingWorkerDummy();
    KeybindingWorkerDummy(FaceTrackNoIR& w, Key keyCenter, Key keyInhibit, Key keyStartStop, Key keyZero);
	void run();
};
#else
class KeybindingWorkerDummy {
public:
    KeybindingWorkerDummy(FaceTrackNoIR& w, Key keyCenter, Key keyInhibit, Key keyStartStop, Key keyZero);
	void run() {}
};
#endif

class KeybindingWorker : public QThread, public KeybindingWorkerDummy {
	Q_OBJECT
public:
	KeybindingWorker(FaceTrackNoIR& w, Key keyCenter, Key keyInhibit, Key keyStartStop, Key keyZero) : KeybindingWorkerDummy(w, keyCenter, keyInhibit, keyStartStop, keyZero)
	{
	}
	void run() {
		KeybindingWorkerDummy::run();
	}


};
