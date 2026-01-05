#include "key.hpp"

bool Key::should_process()
{
    if (!enabled || (keycode == 0 && guid.isEmpty()))
        return false;
    bool ret = !held || timer.elapsed_ms() > 100;
    timer.start();
    return ret;
}

Key::Key() = default;
