if(WIN32)
    opentrack_boilerplate(opentrack-dinput BIN)
    target_link_libraries(opentrack-dinput dinput8)
endif()
