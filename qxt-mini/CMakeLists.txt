if(UNIX OR APPLE)
    opentrack_boilerplate(opentrack-qxt-mini NO-COMPAT)
    if(NOT APPLE)
        target_link_libraries(opentrack-qxt-mini X11)
    else()
        set_target_properties(opentrack-qxt-mini PROPERTIES LINK_FLAGS
            "-framework Carbon -framework CoreFoundation")
    endif()
endif()
