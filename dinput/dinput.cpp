#include "dinput.hpp"
#include <QDebug>

int di_t::refcnt{0};
diptr di_t::handle;
QMutex di_t::lock;

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
    QMutexLocker l(&lock);

    if (!handle)
        handle = init_di_();

    ++refcnt;
}

void di_t::unref_di()
{
    QMutexLocker l(&lock);

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

