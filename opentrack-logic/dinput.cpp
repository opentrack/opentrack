#ifdef _WIN32

#include "dinput.hpp"
#include <QDebug>

dinput_handle dinput_handle::self;

dinput_handle::dinput_handle() : handle(init_di())
{
}

dinput_handle::~dinput_handle()
{
    if (handle)
    {
        handle->Release();
        handle = nullptr;
    }
}

dinput_handle::di_t dinput_handle::init_di()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
        qDebug() << "dinput: failed CoInitializeEx" << hr << GetLastError();

    static LPDIRECTINPUT8 di_ = nullptr;
    if (di_ == nullptr)
    {
        if (SUCCEEDED(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&di_, NULL)))
        {
            qDebug() << "made dinput8 handle";
            return di_;
        }
        else
        {
            return di_ = nullptr;
        }
    }
    return di_;
}

dinput_handle::di_t dinput_handle::make_di()
{
    return self.handle;
}

#endif
