#undef NDEBUG

#include "dinput.hpp"
#include "compat/macros.hpp"

#include <cassert>
#include <cstdlib>
#include <dinput.h>

#include <QDebug>

diptr di_t::handle;
QMutex di_t::lock;

diptr di_t::init_di()
{
    QMutexLocker l(&lock);

    CoInitialize(nullptr);

    if (!handle)
        handle = init_di_();

    return handle;
}

diptr di_t::operator->() const
{
    return init_di();
}

di_t::operator bool() const
{
    return !!init_di();
}

di_t::operator diptr() const
{
    return init_di();
}

diptr di_t::init_di_()
{
    diptr di = nullptr;
    HRESULT hr = DirectInput8Create(GetModuleHandle(nullptr),
                                    DIRECTINPUT_VERSION,
                                    IID_IDirectInput8,
                                    (void**)&di,
                                    nullptr);
    if (!SUCCEEDED(hr))
    {
        qDebug() << "can't make dinput:" << (void*)(LONG_PTR)hr;
        qDebug() << "crashing!";
        std::abort();
    }

    //qDebug() << "dinput: initialized";

    return di;
}

di_t::di_t() = default;

bool di_t::poll_device(IDirectInputDevice8A* dev)
{
    HRESULT hr;
    assert(handle);

    switch (dev->Poll())
    {
    case DI_OK:
    case DI_NOEFFECT:
        return true;
    default:
        break;
    }

    switch (hr = dev->Acquire())
    {
    default:
        break;
    case DI_OK:
    case S_FALSE:
        switch (hr = dev->Poll())
        {
        case DI_OK:
        case DI_NOEFFECT:
            return true;
        default:
            break;
        }
        break;
    }

    eval_once(qDebug() << "dinput: device poll failed:" << (void*)hr);

    return false;
}
