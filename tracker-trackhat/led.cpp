#include "trackhat.hpp"

namespace trackhat_impl {

void led_updater::update_(trackHat_Device_t* device, trackHat_SetLeds_t leds)
{
    if (leds_.ledRedState == leds.ledRedState &&
        leds_.ledGreenState == leds.ledGreenState &&
        leds_.ledBlueState == leds.ledBlueState)
        return;
    (void)trackHat_SetLeds(device, &leds);
    leds_ = leds;
}

trackHat_SetLeds_t led_updater::next_state(led_mode mode, led_state new_state)
{
    switch (mode)
    {
    case led_mode::off:
        state_ = led_state::stopped;
        timer_ = std::nullopt;
        return LED_off;
    default:
    case led_mode::constant:
        state_ = led_state::stopped;
        timer_ = std::nullopt;
        return LED_idle;
    case led_mode::dynamic:
        break;
    }

    if (new_state <= led_state::stopped)
    {
        state_ = new_state;
        timer_ = std::nullopt;
        return LED_idle;
    }
    else if (new_state == state_)
    {
        timer_ = std::nullopt;
        return leds_;
    }
    else if (!timer_)
    {
        timer_ = Timer{};
        return leds_;
    }
    else if (timer_->elapsed_ms() > SWITCH_TIME_MS)
    {
        state_ = new_state;
        timer_ = std::nullopt;
        return new_state == led_state::not_tracking
               ? LED_not_tracking
               : LED_tracking;
    }
    else
        return leds_;
}

void led_updater::update(trackHat_Device_t* device, led_mode mode, led_state new_state)
{
    auto leds = next_state(mode, new_state);
    update_(device, leds);
}

} // namespace trackhat_impl
