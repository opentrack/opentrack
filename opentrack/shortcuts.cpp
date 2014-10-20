#include "shortcuts.h"

KeyboardShortcutDialog::KeyboardShortcutDialog()
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    for ( int i = 0; i < global_key_sequences.size(); i++) {
        ui.cbxCenterKey->addItem(global_key_sequences.at(i));
        ui.cbxToggleKey->addItem(global_key_sequences.at(i));
    }

    tie_setting(s.center.key_index, ui.cbxCenterKey);
    tie_setting(s.center.alt, ui.chkCenterAlt);
    tie_setting(s.center.shift, ui.chkCenterShift);
    tie_setting(s.center.ctrl, ui.chkCenterCtrl);

    tie_setting(s.toggle.key_index, ui.cbxToggleKey);
    tie_setting(s.toggle.alt, ui.chkToggleAlt);
    tie_setting(s.toggle.shift, ui.chkToggleShift);
    tie_setting(s.toggle.ctrl, ui.chkToggleCtrl);
}

void KeyboardShortcutDialog::doOK() {
    s.b->save();
    this->close();
    emit reload();
}

void KeyboardShortcutDialog::doCancel() {
    s.b->reload();
    close();
}

#if defined(_WIN32)
#include <windows.h>

KeybindingWorkerImpl::~KeybindingWorkerImpl() {
    should_quit = true;
    wait();
    if (dinkeyboard) {
        dinkeyboard->Unacquire();
        dinkeyboard->Release();
    }
    if (din)
        din->Release();
}

KeybindingWorkerImpl::KeybindingWorkerImpl(Key keyCenter, Key keyToggle)
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

        if (key->shift && !shift) return false;
        if (key->ctrl && !ctrl) return false;
        if (key->alt && !alt) return false;

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

void Shortcuts::bind_keyboard_shortcut(K &key, key_opts& k)
{
#if !defined(_WIN32)
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
#else
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
#endif
}