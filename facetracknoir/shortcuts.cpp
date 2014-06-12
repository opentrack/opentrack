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
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    for ( int i = 0; i < global_key_sequences.size(); i++) {
        ui.cbxCenterKey->addItem(global_key_sequences.at(i));
        ui.cbxToggleKey->addItem(global_key_sequences.at(i));
    }

    tie_setting(mainApp->s.center_key.key_index, ui.cbxCenterKey);
    tie_setting(mainApp->s.center_key.alt, ui.chkCenterAlt);
    tie_setting(mainApp->s.center_key.shift, ui.chkCenterShift);
    tie_setting(mainApp->s.center_key.ctrl, ui.chkCenterCtrl);

    tie_setting(mainApp->s.toggle_key.key_index, ui.cbxToggleKey);
    tie_setting(mainApp->s.toggle_key.alt, ui.chkToggleAlt);
    tie_setting(mainApp->s.toggle_key.shift, ui.chkToggleShift);
    tie_setting(mainApp->s.toggle_key.ctrl, ui.chkToggleCtrl);
}

//
// OK clicked on server-dialog
//
void KeyboardShortcutDialog::doOK() {
    mainApp->b->save();
    this->close();
    if (mainApp->tracker)
        mainApp->bindKeyboardShortcuts();
}

void KeyboardShortcutDialog::doCancel() {
    mainApp->b->revert();
    close();
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
: din(0), dinkeyboard(0), kCenter(keyCenter), kToggle(keyToggle), window(w), should_quit(true)
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
