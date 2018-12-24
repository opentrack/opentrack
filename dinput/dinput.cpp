#include "dinput.hpp"
#include <QDebug>

std::atomic<int> di_t::refcnt;
std::atomic_flag di_t::init_lock = ATOMIC_FLAG_INIT;
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
    while (init_lock.test_and_set())
        (void)0;

    if (!handle)
        handle = init_di_();

    ++refcnt;

    init_lock.clear();
}

void di_t::unref_di()
{
    const int refcnt_ = --refcnt;

    if (refcnt_ == 0)
    {
        while (init_lock.test_and_set())
            (void)0;

        qDebug() << "exit: di handle";
        handle->Release();

        init_lock.clear();
    }
}

di_t::~di_t()
{
    unref_di();
}

