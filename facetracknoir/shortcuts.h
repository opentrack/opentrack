#pragma once
#include <QWidget>
#include <QElapsedTimer>
#include <QThread>
#include <QMessageBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSettings>
#include "ui_ftnoir_keyboardshortcuts.h"

class FaceTrackNoIR;

class KeyboardShortcutDialog: public QWidget
{
    Q_OBJECT
public:

    KeyboardShortcutDialog( FaceTrackNoIR *ftnoir, QWidget *parent );
private:
	Ui::UICKeyboardShortcutDialog ui;
	FaceTrackNoIR *mainApp;

private slots:
	void doOK();
	void doCancel();
};

extern QList<QString> global_key_sequences;

#if defined(_WIN32)
extern QList<int> global_windows_key_sequences;
#   undef DIRECTINPUT_VERSION
#   define DIRECTINPUT_VERSION 0x0800
#   include <windows.h>
#   include <dinput.h>

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

#if defined(_WIN32)
class KeybindingWorkerImpl {
private:
    LPDIRECTINPUT8 din;
    LPDIRECTINPUTDEVICE8 dinkeyboard;
    Key kCenter;
    Key kToggle;
    FaceTrackNoIR& window;
public:
    volatile bool should_quit;
    ~KeybindingWorkerImpl();
    KeybindingWorkerImpl(FaceTrackNoIR& w, Key keyCenter, Key keyToggle);
	void run();
};
#else
class KeybindingWorkerImpl {
public:
    KeybindingWorkerImpl(FaceTrackNoIR& w, Key keyCenter, Key keyToggle);
	void run() {}
};
#endif

class KeybindingWorker : public QThread, public KeybindingWorkerImpl {
	Q_OBJECT
public:
    KeybindingWorker(FaceTrackNoIR& w, Key keyCenter, Key keyToggle) : KeybindingWorkerImpl(w, keyCenter, keyToggle)
	{
	}
	void run() {
		KeybindingWorkerImpl::run();
	}
};
