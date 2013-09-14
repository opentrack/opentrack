#include "facetracknoir/facetracknoir.h"
#include "facetracknoir/shortcuts.h"

KeyboardShortcutDialog::KeyboardShortcutDialog( FaceTrackNoIR *ftnoir, QWidget *parent, Qt::WindowFlags f ) :
QWidget( parent , f)
{
    ui.setupUi( this );

    QPoint offsetpos(100, 100);
    this->move(parent->pos() + offsetpos);

    mainApp = ftnoir;											// Preserve a pointer to FTNoIR

    // Connect Qt signals to member-functions
    connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
    connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));

    for ( int i = 0; i < global_key_sequences.size(); i++) {
        ui.cbxCenterKey->addItem(global_key_sequences.at(i));
    }

    loadSettings();
}

//
// Destructor for server-dialog
//
KeyboardShortcutDialog::~KeyboardShortcutDialog() {
    qDebug() << "~KeyboardShortcutDialog() says: started";
}

//
// OK clicked on server-dialog
//
void KeyboardShortcutDialog::doOK() {
    save();
    this->close();
    mainApp->bindKeyboardShortcuts();
}

// override show event
void KeyboardShortcutDialog::showEvent ( QShowEvent * event ) {
    loadSettings();
}

//
// Cancel clicked on server-dialog
//
void KeyboardShortcutDialog::doCancel() {
    //
    // Ask if changed Settings should be saved
    //
    if (settingsDirty) {
        int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Discard );

        qDebug() << "doCancel says: answer =" << ret;

        switch (ret) {
            case QMessageBox::Save:
                save();
                this->close();
                break;
            case QMessageBox::Discard:
                this->close();
                break;
            case QMessageBox::Cancel:
                // Cancel was clicked
                break;
            default:
                // should never be reached
            break;
        }
    }
    else {
        this->close();
    }
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void KeyboardShortcutDialog::loadSettings() {
    qDebug() << "loadSettings says: Starting ";
    QSettings settings("opentrack");	// Registry settings (in HK_USER)

    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
    QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

    qDebug() << "loadSettings says: iniFile = " << currentFile;

    iniFile.beginGroup ( "KB_Shortcuts" );

    ui.chkCenterShift->setChecked (iniFile.value ( "Shift_Center", 0 ).toBool());
    ui.chkCenterCtrl->setChecked (iniFile.value ( "Ctrl_Center", 0 ).toBool());
    ui.chkCenterAlt->setChecked (iniFile.value ( "Alt_Center", 0 ).toBool());

    ui.cbxCenterKey->setCurrentIndex(iniFile.value("Key_index_Center", 0).toInt());

    iniFile.endGroup ();

    settingsDirty = false;

}

//
// Save the current Settings to the currently 'active' INI-file.
//
void KeyboardShortcutDialog::save() {

    qDebug() << "save() says: started";

    QSettings settings("opentrack");	// Registry settings (in HK_USER)

    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
    QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

    iniFile.beginGroup ( "KB_Shortcuts" );
    iniFile.setValue ( "Key_index_Center", ui.cbxCenterKey->currentIndex() );
    iniFile.setValue ( "Shift_Center", ui.chkCenterShift->isChecked() );
    iniFile.setValue ( "Ctrl_Center", ui.chkCenterCtrl->isChecked() );
    iniFile.setValue ( "Alt_Center", ui.chkCenterAlt->isChecked() );

    iniFile.endGroup ();

    settingsDirty = false;
}

#if defined(__WIN32) || defined(_WIN32)
#include <windows.h>

KeybindingWorkerDummy::~KeybindingWorkerDummy() {
    if (dinkeyboard) {
        dinkeyboard->Unacquire();
        dinkeyboard->Release();
    }
    if (din)
        din->Release();
}

KeybindingWorkerDummy::KeybindingWorkerDummy(FaceTrackNoIR& w, Key keyCenter)
: kCenter(keyCenter), window(w), should_quit(true), din(0), dinkeyboard(0)
{
    if (DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&din, NULL) != DI_OK) {
        qDebug() << "setup DirectInput8 Creation failed!" << GetLastError();
        return;
    }
    if (din->CreateDevice(GUID_SysKeyboard, &dinkeyboard, NULL) != DI_OK) {
        din->Release();
        din = 0;
        qDebug() << "setup CreateDevice function failed!" << GetLastError();
        return;
    }
    if (dinkeyboard->SetDataFormat(&c_dfDIKeyboard) != DI_OK) {
        qDebug() << "setup SetDataFormat function failed!" << GetLastError();
        dinkeyboard->Release();
        dinkeyboard = 0;
        din->Release();
        din = 0;
        return;
    }

    if (dinkeyboard->SetCooperativeLevel(window.winId(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK) {
        dinkeyboard->Release();
        din->Release();
        din = 0;
        dinkeyboard = 0;
        qDebug() << "setup SetCooperativeLevel function failed!" << GetLastError();
        return;
    }
    if (dinkeyboard->Acquire() != DI_OK)
    {
        dinkeyboard->Release();
        din->Release();
        din = 0;
        dinkeyboard = 0;
        qDebug() << "setup dinkeyboard Acquire failed!" << GetLastError();
        return;
    }
    should_quit = false;
}

#define PROCESS_KEY(k, s) \
    if (isKeyPressed(&k, keystate) && (!k.ever_pressed ? (k.timer.start(), k.ever_pressed = true) : k.timer.restart() > 100)) \
        window.s();

static bool isKeyPressed( const Key *key, const BYTE *keystate ) {
    bool shift;
    bool ctrl;
    bool alt;

    if (keystate[key->keycode] & 0x80) {
        shift = ( (keystate[DIK_LSHIFT] & 0x80) || (keystate[DIK_RSHIFT] & 0x80) );
        ctrl  = ( (keystate[DIK_LCONTROL] & 0x80) || (keystate[DIK_RCONTROL] & 0x80) );
        alt   = ( (keystate[DIK_LALT] & 0x80) || (keystate[DIK_RALT] & 0x80) );

        //
        // If one of the modifiers is needed and not pressed, return false.
        //
        if (key->shift && !shift) return false;
        if (key->ctrl && !ctrl) return false;
        if (key->alt && !alt) return false;

        //
        // All is well!
        //
        return true;
    }
    return false;
}

void KeybindingWorkerDummy::run() {
    BYTE keystate[256];
    while (!should_quit)
    {
        if (dinkeyboard->GetDeviceState(256, (LPVOID)keystate) != DI_OK) {
            qDebug() << "Tracker::run GetDeviceState function failed!" << GetLastError();
            Sleep(25);
            continue;
        }

        PROCESS_KEY(kCenter, shortcutRecentered);

        Sleep(25);
    }
}
#else
#endif
