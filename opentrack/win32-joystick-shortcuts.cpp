#include "win32-joystick-shortcuts.hpp"

LPDIRECTINPUT8& win32_joy_ctx::dinput_handle()
{
    (void) CoInitialize(nullptr);
    
    static LPDIRECTINPUT8 dinput_handle_ = nullptr;
    
    if (dinput_handle_ == nullptr)
        (void) DirectInput8Create(GetModuleHandle(nullptr),
                                  DIRECTINPUT_VERSION,
                                  IID_IDirectInput8,
                                  (void**) &dinput_handle_,
                                  nullptr);
    
    return dinput_handle_;
}

std::unordered_map<QString, std::shared_ptr<win32_joy_ctx::joy>>& win32_joy_ctx::joys()
{
    static std::unordered_map<QString, std::shared_ptr<joy>> js;
    
    return js;
}

win32_joy_ctx& win32_joy_ctx::make()
{
    static win32_joy_ctx ret;
    return ret;
}
