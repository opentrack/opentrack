otr_module(tracker-smoothtrack)
find_package(PkgConfig REQUIRED)
pkg_check_modules(USBMUXD REQUIRED IMPORTED_TARGET libusbmuxd-2.0)
target_link_libraries(${self} PRIVATE PkgConfig::USBMUXD)
