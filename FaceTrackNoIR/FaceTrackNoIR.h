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

#include <tchar.h>
#include <QtGui/QMainWindow>
#include <QApplication>
#include <QFileDialog>
#include <QListView>
#include <QPainter>
#include <QWidget>
#include <QDialog>
#include <QUrl>

#include "../FTNoIR_PoseWidget/glwidget.h"

#include "ui_FaceTrackNoIR.h"
#include "ui_FTNoIR_KeyboardShortcuts.h"
#include "ui_FTNoIR_Preferences.h"
#include "ui_FTNoIR_Curves.h"

//
#include "..\ftnoir_protocol_base\FTNoIR_Protocol_base.h"
#include "..\ftnoir_tracker_base\FTNoIR_Tracker_base.h"
#include "AutoClosePtr.h"

// 1a. COM-Like usage with smart pointer.
// No need to call `ITracker::Release'; the instance will
// be released automatically in destructor of the smart pointer.
typedef AutoClosePtr<ITrackerDialog, void, &ITrackerDialog::Release> ITrackerDialogPtr;
typedef ITrackerDialog *(WINAPI *importGetTrackerDialog)(void);
typedef AutoClosePtr<IProtocolDialog, void, &IProtocolDialog::Release> IProtocolDialogPtr;
typedef IProtocolDialog *(WINAPI *importGetProtocolDialog)(void);


#include <Dshow.h>

class Tracker;				// pre-define class to avoid circular includes

void getCurvePoints(QSettings *iniFile, QString prefix, QPointF *one, QPointF *two, QPointF *three, QPointF *four, 
					int NeutralZone, int Sensitivity, int MaxInput, int MaxOutput);

class FaceTrackNoIR : public QMainWindow
{
	Q_OBJECT

public:
	FaceTrackNoIR(QWidget *parent = 0, Qt::WFlags flags = 0);
	~FaceTrackNoIR();

	void getGameProgramName();					// Get the ProgramName from the game and display it.
	void updateSettings();						// Update the settings (let Tracker read INI-file).

	QFrame *getVideoWidget();					// Get a pointer to the video-widget, to use in the DLL

private:
	Ui::FaceTrackNoIRClass ui;
	Tracker *tracker;
	QTimer *timMinimizeFTN;						// Timer to Auto-minimize
	QTimer *timUpdateHeadPose;					// Timer to display headpose
	QStringList iniFileList;					// List of INI-files, that are present in the Settings folder

	ITrackerDialogPtr pTrackerDialog;			// Pointer to Tracker dialog instance (in DLL)
	IProtocolDialogPtr pProtocolDialog;			// Pointer to Protocol dialog instance (in DLL)

	/** Widget variables **/
	QVBoxLayout *l;
	QWidget *_engine_controls;
	QWidget *_server_controls;
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

		void setIcon(int index);
		void iconActivated(QSystemTrayIcon::ActivationReason reason);
		void trackingSourceSelected(int index);
		void profileSelected(int index);

		void showVideoWidget();
		void showHeadPoseWidget();
		void showEngineControls();
		void showServerControls();
		void showPreferences();
		void showKeyboardShortcuts();
		void showCurveConfiguration();

		// invert checkboxes
		void setInvertYaw( int invert );
		void setInvertPitch( int invert );
		void setInvertRoll( int invert );
		void setInvertX( int invert );
		void setInvertY( int invert );
		void setInvertZ( int invert );

		void setUseFilter( int set );
		void showHeadPose();

		// EWMA filter sliders
		void setMinSmooth( int redh );
		void setMaxSmooth( int redh );
		void setPowCurve( int redh );

		//smoothing slider
		void setSmoothing( int smooth );

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
	QList<QString> stringList;			// List of strings, that describe the keyboard-keys
	QList<BYTE> keyList; 				// List of keys, with the values of the keyboard-keys

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
	void curveChanged( bool change ) { settingsDirty = true; };
};


#endif // FaceTrackNoIR_H
