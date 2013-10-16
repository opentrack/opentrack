#include "facetracknoir/facetracknoir.h"
#include "facetracknoir/shortcuts.h"

KeyboardShortcutDialog::KeyboardShortcutDialog( FaceTrackNoIR *ftnoir, QWidget *parent )
    : QWidget( parent, Qt::Dialog)
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
        ui.cbxToggleKey->addItem(global_key_sequences.at(i));
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

void KeyboardShortcutDialog::loadSettings() {
    QSettings settings("opentrack");	// Registry settings (in HK_USER)

    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
    QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

    qDebug() << "loadSettings says: iniFile = " << currentFile;

    iniFile.beginGroup ( "KB_Shortcuts" );

    const char* names[] = {
        "Center", "Toggle"
    };

    QComboBox* checkboxen[] = {
        ui.cbxCenterKey,
        ui.cbxToggleKey
    };

    QCheckBox* boxen[2][3] = {
        {
            ui.chkCenterShift,
            ui.chkCenterCtrl,
            ui.chkCenterAlt
        },
        {
            ui.chkToggleShift,
            ui.chkToggleCtrl,
            ui.chkToggleAlt
        }
    };

    const char* modnames[] = {
        "Shift", "Ctrl", "Alt"
    };

    const char* keynames[] = {
        "Center", "Toggle"
    };

    const int KEY_COUNT = 2;
    const int MODIFIERS = 3;

    for (int i = 0; i < KEY_COUNT; i++)
    {
        for (int m = 0; i < MODIFIERS; i++)
        {
            boxen[i][m]->setChecked (iniFile.value ( modnames[m] + QString("_") + QString(keynames[i]), 0 ).toBool());
        }
        checkboxen[i]->setCurrentIndex(iniFile.value("Key_index_" + QString(names[i]), 0).toInt());
    }

    iniFile.endGroup ();

    settingsDirty = false;
}

void KeyboardShortcutDialog::save() {
    const char* keynames[] = {
        "Center", "Toggle"
    };

    QComboBox* checkboxen[] = {
        ui.cbxCenterKey,
        ui.cbxToggleKey
    };

    const char* modnames[] = {
        "Shift", "Ctrl", "Alt"
    };

    QCheckBox* boxen[2][3] = {
        {
            ui.chkCenterShift,
            ui.chkCenterCtrl,
            ui.chkCenterAlt
        },
        {
            ui.chkToggleShift,
            ui.chkToggleCtrl,
            ui.chkToggleAlt
        }
    };

    const int MODIFIERS = 3;
    const int KEY_COUNT = 2;

    QSettings settings("opentrack");	// Registry settings (in HK_USER)

    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
    QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

    iniFile.beginGroup ( "KB_Shortcuts" );

    for (int i = 0; i < KEY_COUNT; i++)
    {
        iniFile.setValue ( "Key_index_" + QString(keynames[i]),
                           checkboxen[i]->currentIndex() );
        for (int m = 0; i < MODIFIERS; i++)
        {
            iniFile.setValue(modnames[m] + QString("_") + keynames[i], !!boxen[i][m]->isChecked());
        }
    }

    iniFile.endGroup();

    settingsDirty = false;
}

#if defined(_WIN32)
#include <windows.h>

KeybindingWorkerImpl::~KeybindingWorkerImpl() {
    if (dinkeyboard) {
        dinkeyboard->Unacquire();
        dinkeyboard->Release();
    }
    if (din)
        din->Release();
}

KeybindingWorkerImpl::KeybindingWorkerImpl(FaceTrackNoIR& w, Key keyCenter, Key keyToggle)
: din(0), dinkeyboard(0), kCenter(keyCenter), ktoggle(keyTogle), window(w), should_quit(true)
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

    if (dinkeyboard->SetCooperativeLevel((HWND) window.winId(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK) {
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

#define PROCESS_KEY(k, s) \
    if (isKeyPressed(&k, keystate) && (!k.ever_pressed ? (k.timer.start(), k.ever_pressed = true) : k.timer.restart() > 100)) \
        window.s();

void KeybindingWorkerImpl::run() {
    BYTE keystate[256];
    while (!should_quit)
    {
        if (dinkeyboard->GetDeviceState(256, (LPVOID)keystate) != DI_OK) {
            qDebug() << "Tracker::run GetDeviceState function failed!" << GetLastError();
            Sleep(25);
            continue;
        }

        PROCESS_KEY(kCenter, shortcutRecentered);
        PROCESS_KEY(kToggle, shortcutToggled);

        Sleep(25);
    }
}

#else
#endif
