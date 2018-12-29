#include "dinput.hpp"
#include "compat/spinlock.hpp"
#include <QDebug>

int di_t::refcnt{0};
std::atomic_flag di_t::lock = ATOMIC_FLAG_INIT;
diptr di_t::handle;

diptr di_t::init_di_()
{
    CoInitialize(nullptr);

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

    return di;
}

di_t::di_t()
{
    ref_di();
}

void di_t::ref_di()
{
    spinlock_guard l(lock);

    if (!handle)
        handle = init_di_();

    ++refcnt;
}

void di_t::unref_di()
{
    spinlock_guard l(lock);

    const int refcnt_ = --refcnt;

    if (refcnt_ == 0)
    {
        qDebug() << "exit: di handle";
        handle->Release();
    }
}

di_t::~di_t()
{
    unref_di();
}

