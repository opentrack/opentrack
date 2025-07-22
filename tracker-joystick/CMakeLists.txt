if(WIN32)
    otr_module(tracker-joystick)
    target_link_libraries(opentrack-tracker-joystick PRIVATE opentrack-input)
endif()
