# libusbmuxd and its dependencies come from the opentrack-usbmux submodule,
# built here as one LGPL shared library and installed next to the plugin.
# Without the submodule, fall back to a system libusbmuxd (Linux distros).
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/opentrack-usbmux/CMakeLists.txt")
    add_subdirectory(opentrack-usbmux)
    otr_module(tracker-smoothtrack)
    target_link_libraries(${self} PRIVATE opentrack-usbmux)
    if(APPLE)
        # make-app-bundle.sh has macdeployqt pick up dylibs from Library/.
        install(TARGETS opentrack-usbmux LIBRARY DESTINATION "${opentrack-bin}/Library")
    else()
        install(TARGETS opentrack-usbmux
                RUNTIME DESTINATION "${opentrack-libexec}"
                LIBRARY DESTINATION "${opentrack-libexec}"
                PERMISSIONS ${opentrack-perms-exec})
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
