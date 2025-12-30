#if defined _WIN32 && OPENTRACK_HAS_GAMEINPUT
#include "input/gameinput.hpp"
#include "input/key-opts.hpp"
#include "input/key.hpp"

#include <cstring>
#include <cstdint>
#include <utility>
#include <memory>
#include <thread>
#include <shared_mutex>
#include <optional>
#include <array>
#include <bitset>

#include <QLibrary>

#include <my-gameinput/GameInput.h>
#include <wrl/client.h>

namespace opentrack::gameinput {

using Microsoft::WRL::ComPtr;
using device_id = APP_LOCAL_DEVICE_ID;
using device = ComPtr<IGameInputDevice>;
using keybinding = ::Key;
using handler = std::function<void(const keybinding&)>;
using key_tuple = std::tuple<const key_opts*, handler, bool>;

namespace {

uint32_t fnv1a_32(const std::uint8_t* data, std::uint32_t length)
{
    constexpr std::uint32_t FNV_prime = 16777619;
    constexpr std::uint32_t offset_basis = 2166136261;
    uint32_t hash = offset_basis;

    for (auto i = 0u; i < length; ++i)
    {
        hash ^= data[i];
        hash *= FNV_prime;
    }
    return hash;
}

struct comparator
{
    bool operator()(const APP_LOCAL_DEVICE_ID& a, const APP_LOCAL_DEVICE_ID& b) const noexcept
    {
        return !std::memcmp(a.value, b.value, APP_LOCAL_DEVICE_ID_SIZE);
    }
};

struct hasher
{
    size_t operator()(const APP_LOCAL_DEVICE_ID& x) const noexcept
    {
        return fnv1a_32(x.value, APP_LOCAL_DEVICE_ID_SIZE);
    }
};

QString make_device_guid(const APP_LOCAL_DEVICE_ID& id)
{
    using namespace Qt::Literals::StringLiterals;
    QString s;
    s.reserve(3 + 1 + APP_LOCAL_DEVICE_ID_SIZE*4/3 + 3);
    s.append("GI!"_L1);
    auto t = QByteArray::fromRawData(reinterpret_cast<const char*>(id.value), APP_LOCAL_DEVICE_ID_SIZE)
                        .toBase64(QByteArray::Base64Encoding | QByteArray::OmitTrailingEquals);
    s.append(t);
    return s;
}

struct Device
{
    ComPtr<IGameInputDevice> dev;
    QString guid;
    std::bitset<64> gamepad_bits;
    std::optional<GameInputKind> supportedInput = GameInputKind::GameInputKindUnknown;

    IGameInputDevice& operator->() const { return *dev.Get(); }
};

auto get_gi_create_ptr()
{
    using gi_create = HRESULT (WINAPI*)(IGameInput**);
    static QLibrary L{"GameInput.dll"};
    static const gi_create funptr = []() {
        if (L.load())
        {
            if (auto p = L.resolve("GameInputCreate"))
                return reinterpret_cast<gi_create>(p);
            else
                qDebug() << "gi: resolve dll symbol failed:" << L.errorString();
        }
        else
            qDebug() << "gi: load failed:" << L.errorString();
        return gi_create{};
    }();
    return funptr;
}

constexpr GameInputGamepadButtons gamepad_buttons[] = {
    GameInputGamepadNone,
    GameInputGamepadMenu,
    GameInputGamepadView,
    GameInputGamepadA,
    GameInputGamepadB,
    GameInputGamepadX,
    GameInputGamepadY,
    GameInputGamepadDPadUp,
    GameInputGamepadDPadDown,
    GameInputGamepadDPadLeft,
    GameInputGamepadDPadRight,
    GameInputGamepadLeftShoulder,
    GameInputGamepadRightShoulder,
    GameInputGamepadLeftThumbstick,
    GameInputGamepadRightThumbstick,
};

} // namespace

Worker::Worker() = default;
Worker::~Worker() noexcept = default;

class Worker_ final : public Worker
{
    std::recursive_mutex _lock;
    GameInputCallbackToken _device_cancellation_token {};
    ComPtr<IGameInput> gi;
    std::unordered_map<device_id, Device, hasher, comparator> _devices{};
    std::bitset<std::size(gamepad_buttons)> button_state;

    void add_device(IGameInputDevice* dev);
    void remove_device(IGameInputDevice* dev);
    bool should_skip_device(GameInputKind kind);

    static void CALLBACK DeviceCallback_stub(
        GameInputCallbackToken callbackToken,
        void* context,
        IGameInputDevice* device,
        uint64_t timestamp,
        GameInputDeviceStatus currentStatus,
        GameInputDeviceStatus previousStatus);

    void device_callback(
        GameInputCallbackToken callback_token,
        IGameInputDevice* device,
        uint64_t timestamp,
        GameInputDeviceStatus currentStatus,
        GameInputDeviceStatus previousStatus);

public:
    bool is_ok() const;

    Worker_();
    ~Worker_() noexcept override;

    void init_gameinput();
    void kill_gameinput();

    void poll(const std::function<void(const Key&)>& fn, Mods mods) override;

    friend class Token;
};

bool Worker_::is_ok() const
{
    return gi;
}

void CALLBACK Worker_::DeviceCallback_stub(GameInputCallbackToken callbackToken,
                                           void* context,
                                           IGameInputDevice* device,
                                           uint64_t timestamp,
                                           GameInputDeviceStatus currentStatus,
                                           GameInputDeviceStatus previousStatus)
{
    auto* ptr = static_cast<Worker_*>(context);
    ptr->device_callback(callbackToken, device, timestamp, currentStatus, previousStatus);
}

Worker_::~Worker_() noexcept
{
    kill_gameinput();
}

Worker_::Worker_()
{
    init_gameinput();
    _devices.reserve(64);
    _devices.max_load_factor(0.4f);
}

void Worker_::init_gameinput()
{
    (void)CoInitialize(nullptr);
    kill_gameinput();

    auto GI_create = get_gi_create_ptr();

    if (!GI_create)
        return;

    if (HRESULT hr = GI_create(gi.GetAddressOf()); hr != S_OK)
    {
        eval_once(qDebug() << "gi: GameInputCreate error" << (void*)(std::intptr_t)hr);
        return;
    }

    gi->SetFocusPolicy(GameInputDefaultFocusPolicy);

    constexpr GameInputKind mask =
        //GameInputKindUiNavigation |
        //GameInputKindRawDeviceReport |
        //GameInputKindControllerAxis |
        GameInputKindControllerButton |
        GameInputKindControllerSwitch |
        GameInputKindKeyboard |
        GameInputKindMouse |
        GameInputKindTouch |
        //GameInputKindMotion |
        GameInputKindArcadeStick |
        GameInputKindFlightStick |
        GameInputKindGamepad |
        GameInputKindRacingWheel;

    auto hr = gi->RegisterDeviceCallback(nullptr, mask,
                                         GameInputDeviceConnected,
                                         GameInputBlockingEnumeration,
                                         this, DeviceCallback_stub,
                                         &_device_cancellation_token);

    if (!SUCCEEDED(hr))
    {
        eval_once(qDebug() << "gi: RegisterDeviceCallback error"
                           << (void*)(std::intptr_t)hr
                           << (void*)(std::intptr_t)GetLastError());
        gi = nullptr;
        return;
    }
}

void Worker_::kill_gameinput()
{
    std::lock_guard l{_lock};

    if (gi && _device_cancellation_token)
    {
        bool ret = gi->UnregisterCallback(_device_cancellation_token, (std::uint64_t)-1);
        if (!ret)
            eval_once(qDebug() << "gi: UnregisterCallback error" << (void*)(std::uintptr_t)GetLastError());
    }

    _device_cancellation_token = {};
    gi = nullptr;
}

void Worker_::device_callback(GameInputCallbackToken,
                              IGameInputDevice* device,
                              uint64_t,
                              GameInputDeviceStatus currentStatus,
                              GameInputDeviceStatus previousStatus)
{
    bool is_connected = currentStatus & GameInputDeviceConnected,
         was_connected = previousStatus & GameInputDeviceConnected;

    std::scoped_lock l{_lock};

    if(auto* info = device->GetDeviceInfo(); should_skip_device(info->supportedInput))
        return;

    if (!is_connected && was_connected)
        remove_device(device);
    else if (is_connected && !was_connected)
        add_device(device);
}

void Worker_::add_device(IGameInputDevice* dev)
{
    auto* info = dev->GetDeviceInfo();
    auto id = info->deviceId;
    _devices[id] = Device{dev, make_device_guid(id), {}, info->supportedInput};
    //qDebug() << "gi: device add" << (void*)(std::intptr_t)info->supportedInput << _devices[id].guid;
}

void Worker_::remove_device(IGameInputDevice* dev)
{
    auto id = dev->GetDeviceInfo()->deviceId;
    //qDebug() << "gi: device rm" << _devices[id].guid;
    _devices.erase(id);
}

bool Worker_::should_skip_device(GameInputKind kind)
{
    return !(kind & GameInputKindGamepad);
}

void Worker_::poll(const std::function<void(const Key&)>& fn, Mods mods)
{
    std::scoped_lock l{_lock};
    for (auto& [_, d] : _devices)
    {
        if (!d.supportedInput)
            if (const auto* ptr = d.dev->GetDeviceInfo())
                d.supportedInput = ptr->supportedInput;
        const auto in = d.supportedInput ? *d.supportedInput : GameInputKindUnknown;

        if(in & GameInputKindGamepad) {
            if(ComPtr<IGameInputReading> r;
               SUCCEEDED(gi->GetCurrentReading(GameInputKindGamepad, d.dev.Get(), r.GetAddressOf()))) {
                if(GameInputGamepadState s; r->GetGamepadState(&s)) {
                    for (unsigned i = 0; i < unsigned{std::size(gamepad_buttons)}; i++)
                    {
                        GameInputGamepadButtons btn = gamepad_buttons[i];
                        bool old_state = button_state[i];
                        bool new_state = s.buttons & btn;

                        if (new_state != old_state)
                        {
                            button_state[i] = new_state;
                            Key k;
                            k.guid = d.guid;
                            k.held = new_state;
                            k.ctrl = mods.ctrl;
                            k.alt = mods.alt;
                            k.shift = mods.shift;
                            k.keycode = (int)i;
                            //qDebug() << "gi: gamepad" << d.guid << "key" << (k.held ? "+" : "-") << k.keycode;
                            fn(k);
                        }
                    }
                }
            }
        }
    }
}

Worker* make_gameinput_worker()
{
    auto* w = new Worker_;
    if (!w->is_ok())
    {
        eval_once(qDebug() << "gameinput: not available");
        delete w;
        return nullptr;
    }
    else
    {
        //eval_once(qDebug() << "gameinput: ok");
        return w;
    }
}

} // namespace opentrack::gameinput
#endif
