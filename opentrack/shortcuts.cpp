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

    tie_setting(s.s_main.tray_enabled, ui.trayp);
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

KeybindingWorker::~KeybindingWorker() {
    should_quit = true;
    wait();
    if (dinkeyboard) {
        dinkeyboard->Unacquire();
        dinkeyboard->Release();
    }
    if (din)
        din->Release();
}

KeybindingWorker::KeybindingWorker(Key keyCenter, Key keyToggle, WId handle) :
    din(0), dinkeyboard(0), kCenter(keyCenter), kToggle(keyToggle), should_quit(true)
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
    if (dinkeyboard->SetCooperativeLevel((HWND) handle, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK) {
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

    if (key->keycode != 0 && keystate[key->keycode] & 0x80)
    {
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
        emit s;

void KeybindingWorker::run() {
    BYTE keystate[256];
    while (!should_quit)
    {
        if (dinkeyboard->GetDeviceState(256, (LPVOID)keystate) != DI_OK) {
            qDebug() << "Tracker::run GetDeviceState function failed!" << GetLastError();
            Sleep(25);
            continue;
        }

        PROCESS_KEY(kCenter, center());
        PROCESS_KEY(kToggle, toggle());

        Sleep(14);
    }
}
#endif

void Shortcuts::bind_keyboard_shortcut(K &key, key_opts& k)
{
#if !defined(_WIN32)
    const int idx = k.key_index;
    if (!key)
        key = std::make_shared<QxtGlobalShortcut>();
    else {
        key->setEnabled(false);
        key->setShortcut(QKeySequence::UnknownKey);
    }
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
            key->setShortcut(QKeySequence::fromString(seq, QKeySequence::PortableText));
            key->setEnabled();
        }
    }
#else
    int idx = k.key_index;
    key.keycode = 0;
    key.shift = key.alt = key.ctrl = 0;
    if (idx > 0 && idx < global_windows_key_sequences.size())
        key.keycode = global_windows_key_sequences[idx];
    key.shift = k.shift;
    key.alt = k.alt;
    key.ctrl = k.ctrl;
#endif
}
void Shortcuts::reload() {
#ifndef _WIN32
    if (keyCenter)
    {
        keyCenter->setShortcut(QKeySequence::UnknownKey);
        keyCenter->setEnabled(false);
    }
    if (keyToggle)
    {
        keyToggle->setShortcut(QKeySequence::UnknownKey);
        keyToggle->setEnabled(false);
    }
#endif
    bind_keyboard_shortcut(keyCenter, s.center);
    bind_keyboard_shortcut(keyToggle, s.toggle);
#ifdef _WIN32
    keybindingWorker = nullptr;
    keybindingWorker = std::make_shared<KeybindingWorker>(keyCenter, keyToggle, handle);
    keybindingWorker->start();
#endif
}
