#ifdef _WIN32

#undef NDEBUG
#include "win32-joystick.hpp"
#include "compat/sleep.hpp"

#include <cstddef>
#include <algorithm>
#include <cmath>
#include <objbase.h>

#include <QDebug>

// XXX how many axis update events can we reasonably get in a short time frame?
DIDEVICEOBJECTDATA win32_joy_ctx::joy::keystate_buffers[win32_joy_ctx::joy::num_buffers];

QMutex win32_joy_ctx::enum_state::mtx;
win32_joy_ctx::enum_state win32_joy_ctx::enumerator;

void win32_joy_ctx::poll(fn const& f)
{
    //refresh(false);

    QMutexLocker l(&enum_state::mtx);

    auto& joys = enumerator.get_joys();

    for (auto& j : joys)
    {
        j.second->poll(f);
    }
}

bool win32_joy_ctx::poll_axis(const QString &guid, int* axes)
{
    QMutexLocker l(&enum_state::mtx);

    for (int k = 0; k < 10; k++)
    {
        if (k > 0)
            enumerator.refresh();

        const joys_t& joys = enumerator.get_joys();
        auto iter = joys.find(guid);

        if (iter == joys.end())
            return false;

        auto& j = iter->second;

        auto& joy_handle = j->joy_handle;
        bool ok = false;
        HRESULT hr;

        if (!FAILED(hr = joy_handle->Poll()))
        {
            ok = true;
        }

        if (!ok && FAILED(hr = joy_handle->Acquire()))
        {
            //qDebug() << "joy acquire failed" << hr;
        }

        if (!ok)
        {
            portable::sleep(25);
            (void) joy_handle->Unacquire();
            continue;
        }

        DIJOYSTATE2 js = {};

        if (FAILED(hr = joy_handle->GetDeviceState(sizeof(js), &js)))
        {
            //qDebug() << "joy get state failed" << guid << hr;
            portable::sleep(50);
            continue;
        }

        const int values[] =
        {
            js.lX,
            js.lY,
            js.lZ,
            js.lRx,
            js.lRy,
            js.lRz,
            js.rglSlider[0],
            js.rglSlider[1]
        };

        for (int i = 0; i < 8; i++)
            axes[i] = values[i];

        return true;
    }

    return false;
}

std::vector<win32_joy_ctx::joy_info> win32_joy_ctx::get_joy_info()
{
    std::vector<joy_info> ret;
    QMutexLocker l(&enum_state::mtx);
    auto& joys = enumerator.get_joys();
    ret.reserve(joys.size());

    for (auto& j : joys)
        ret.push_back(joy_info { j.second->name, j.first });

    std::sort(ret.begin(), ret.end(), [&](const joy_info& fst, const joy_info& snd) { return fst.name < snd.name; });

    return ret;
}

win32_joy_ctx::win32_joy_ctx()
{
    refresh();
}

void win32_joy_ctx::refresh()
{
    QMutexLocker l(&enum_state::mtx);
    enumerator.refresh();
}

QString win32_joy_ctx::guid_to_string(const GUID& guid)
{
    char buf[40] = {0};
    wchar_t szGuidW[40] = {0};

    StringFromGUID2(guid, szGuidW, 40);
    WideCharToMultiByte(0, 0, szGuidW, -1, buf, 40, nullptr, nullptr);

    return QString(buf);
}

using fn = win32_joy_ctx::fn;

void win32_joy_ctx::joy::release()
{
    if (joy_handle)
    {
        (void) joy_handle->Unacquire();
        joy_handle->Release();
        joy_handle = nullptr;
    }
}

bool win32_joy_ctx::joy::poll(fn const& f)
{
    HRESULT hr;

    (void) joy_handle->Acquire();

    if (FAILED(hr = joy_handle->Poll()))
    {
        //qDebug() << "joy acquire failed" << guid << hr;
        (void) joy_handle->Unacquire();
        return false;
    }

    DWORD sz = num_buffers;
    if (FAILED(hr = joy_handle->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), keystate_buffers, &sz, 0)))
    {
        //qDebug() << "joy get state failed" << guid << hr;
        return false;
    }

    for (unsigned k = 0; k < sz; k++)
    {
        const DIDEVICEOBJECTDATA& event = keystate_buffers[k];

        bool is_pov = false;
        int i = -1;

        // redefine since MSVC headers don't define as proper constants

#define POV_HAT_OFFSET(k) \
    (offsetof(DIJOYSTATE, rgdwPOV) + (k) * sizeof(DWORD))

#define BUTTON_OFFSET(k) \
    (offsetof(DIJOYSTATE, rgbButtons) + (k) * sizeof(BYTE))

        switch (event.dwOfs)
        {
        case POV_HAT_OFFSET(0): i = 0; is_pov = true; break;
        case POV_HAT_OFFSET(1): i = 1; is_pov = true; break;
        case POV_HAT_OFFSET(2): i = 2; is_pov = true; break;
        case POV_HAT_OFFSET(3): i = 3; is_pov = true; break;
        default:
            if (event.dwOfs >= BUTTON_OFFSET(0) && event.dwOfs <= BUTTON_OFFSET(127))
            {
                unsigned tmp = event.dwOfs;
                tmp -= BUTTON_OFFSET(0);
                tmp /= BUTTON_OFFSET(1) - BUTTON_OFFSET(0);
                tmp &= 127;
                i = tmp;
            }
            break;
        }

        if (is_pov)
        {
            //qDebug() << "DBG: pov" << i << event.dwData;

            unsigned char pos;
            unsigned pos_ = event.dwData;
            if ((pos_ & 0xffff) == 0xffff)
                pos = 0;
            else if (pos_ == ~0u)
                pos = 0;
            else
            {
                using uc = unsigned char;
                pos = uc(((pos_ / 9000u) % 4u) + 1u);
            }

            const bool state[] =
            {
                pos == 1,
                pos == 2,
                pos == 3,
                pos == 4
            };

            i = 128u + i * 4u;

            for (unsigned j = 0; j < 4; j++)
            {
                //pressed[i] = state[j];
                f(guid, i, state[j]);
            }
        }
        else if (i != -1)
        {
            const bool state = !!(event.dwData & 0x80);
            //qDebug() << "DBG: btn" << i << state;
            //pressed[i] = state;
            f(guid, i, state);
        }

    }

    return true;
}

win32_joy_ctx::enum_state::enum_state() : di(dinput_handle::make_di())
{
}

win32_joy_ctx::enum_state::~enum_state()
{
    QMutexLocker l(&mtx);

    joys = std::unordered_map<QString, std::shared_ptr<joy>>();
}

void win32_joy_ctx::enum_state::refresh()
{
    all.clear();

    if (!di)
    {
        qDebug() << "can't create dinput";
        return;
    }

    HRESULT hr;

    if(FAILED(hr = di->EnumDevices(DI8DEVCLASS_GAMECTRL,
                                   EnumJoysticksCallback,
                                   this,
                                   DIEDFL_ATTACHEDONLY)))
    {
        qDebug() << "failed enum joysticks" << hr;
        return;
    }

    for (auto it = joys.begin(); it != joys.end(); )
    {
        if (std::find_if(all.cbegin(), all.cend(), [&](const QString& guid2) { return it->second->guid == guid2; }) == all.end())
            it = joys.erase(it);
        else
            ++it;
    }
}

const win32_joy_ctx::joys_t& win32_joy_ctx::enum_state::get_joys() const { return joys; }

BOOL CALLBACK win32_joy_ctx::enum_state::EnumJoysticksCallback(const DIDEVICEINSTANCE *pdidInstance, void *pContext)
{
    enum_state& state = *reinterpret_cast<enum_state*>(pContext);
    const QString guid = guid_to_string(pdidInstance->guidInstance);
    const QString name = QString(pdidInstance->tszInstanceName);

    const bool exists = state.joys.find(guid) != state.joys.end();

    state.all.push_back(guid);

    if (exists)
        goto end;

    {
        HRESULT hr;
        LPDIRECTINPUTDEVICE8 h;
        if (FAILED(hr = state.di->CreateDevice(pdidInstance->guidInstance, &h, nullptr)))
        {
            qDebug() << "createdevice" << guid << hr;
            goto end;
        }
        if (FAILED(h->SetDataFormat(&c_dfDIJoystick2)))
        {
            qDebug() << "format";
            h->Release();
            goto end;
        }

        // not a static member - need main() to run for some time first
        static const QWidget fake_window;

        if (FAILED(h->SetCooperativeLevel(reinterpret_cast<HWND>(fake_window.winId()), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
        {
            qDebug() << "coop";
            h->Release();
            goto end;
        }

        {
            DIPROPDWORD dipdw;
            dipdw.dwData = 128;
            dipdw.diph.dwHeaderSize = sizeof(dipdw.diph);
            dipdw.diph.dwHow = DIPH_DEVICE;
            dipdw.diph.dwObj = 0;
            dipdw.diph.dwSize = sizeof(dipdw);

            if (h->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph) != DI_OK)
            {
                qDebug() << "setup joystick buffer mode failed!";
                h->Release();
                goto end;
            }
        }

        if (FAILED(hr = h->EnumObjects(EnumObjectsCallback, h, DIDFT_ALL)))
        {
            qDebug() << "enum-objects";
            h->Release();
            goto end;
        }

        state.joys[guid] = std::make_shared<joy>(h, guid, name);
    }
end:
    return DIENUM_CONTINUE;
}

BOOL CALLBACK win32_joy_ctx::enum_state::EnumObjectsCallback(const DIDEVICEOBJECTINSTANCE *pdidoi, void *ctx)
{
    if (pdidoi->dwType & DIDFT_AXIS)
    {
        DIPROPRANGE diprg = {};
        diprg.diph.dwSize = sizeof( DIPROPRANGE );
        diprg.diph.dwHeaderSize = sizeof( DIPROPHEADER );
        diprg.diph.dwHow = DIPH_BYID;
        diprg.diph.dwObj = pdidoi->dwType;
        diprg.lMax = joy_axis_size;
        diprg.lMin = -joy_axis_size;

        HRESULT hr;

        if (FAILED(hr = reinterpret_cast<LPDIRECTINPUTDEVICE8>(ctx)->SetProperty(DIPROP_RANGE, &diprg.diph)))
        {
            qDebug() << "DIPROP_RANGE" << hr;
            return DIENUM_STOP;
        }
    }

    return DIENUM_CONTINUE;
}

win32_joy_ctx::joy::joy(LPDIRECTINPUTDEVICE8 handle, const QString& guid, const QString &name)
    : joy_handle(handle), guid(guid), name(name)
{
    //qDebug() << "make joy" << guid << name << joy_handle;
}

win32_joy_ctx::joy::~joy()
{
    //qDebug() << "nix joy" << guid;
    release();
}

#endif
