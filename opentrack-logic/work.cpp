#include "work.hpp"


Work::Work(Mappings& m, SelectedLibraries& libs, WId handle) :
    libs(libs),
    tracker(std::make_shared<Tracker>(m, libs)),
    sc(std::make_shared<Shortcuts>()),
    handle(handle),
    keys {
        key_tuple(s.key_center, [&](bool) -> void { tracker->center(); }, true),
        key_tuple(s.key_toggle, [&](bool) -> void { tracker->toggle_enabled(); }, true),
        key_tuple(s.key_zero, [&](bool) -> void { tracker->zero(); }, true),
        key_tuple(s.key_toggle_press, [&](bool x) -> void { tracker->set_toggle(!x); }, false),
        key_tuple(s.key_zero_press, [&](bool x) -> void { tracker->set_zero(x); }, false),
        }
{
    reload_shortcuts();
    tracker->start();
}

void Work::reload_shortcuts()
{
    sc->reload(keys);
}

Work::~Work()
{
    sc = nullptr;
    // order matters, otherwise use-after-free -sh
    tracker = nullptr;
    libs = SelectedLibraries();
}
