# vcpkg ships libusbmuxd with a CMake config package (Windows), while distro
# and Homebrew packages only ship pkg-config metadata. Try both, and skip the
# module rather than failing configure when neither is present.
find_package(unofficial-libusbmuxd CONFIG QUIET)

if(unofficial-libusbmuxd_FOUND)
    otr_module(tracker-smoothtrack)
    target_link_libraries(${self} PRIVATE unofficial::libusbmuxd::libusbmuxd)
    if(WIN32)
        otr_install_lib(unofficial::libusbmuxd::libusbmuxd "${opentrack-libexec}")
        # libusbmuxd pulls in libplist and libimobiledevice-glue. The vcpkg
        # toolchain copies those next to the build output but not into the
        # install tree, so the installed plugin would fail to load. This walks
        # the installed binary's imports and copies what it actually needs.
        # Must come after otr_module(), whose install() rule it runs behind.
        if(COMMAND x_vcpkg_install_local_dependencies)
            x_vcpkg_install_local_dependencies(TARGETS ${self} DESTINATION "${opentrack-libexec}")
        endif()
    endif()
else()
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(USBMUXD IMPORTED_TARGET libusbmuxd-2.0)
    endif()
    if(USBMUXD_FOUND)
        otr_module(tracker-smoothtrack)
        target_link_libraries(${self} PRIVATE PkgConfig::USBMUXD)
    endif()
endif()
