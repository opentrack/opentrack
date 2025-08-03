if(LINUX)
    otr_module(tracker-linux-joystick)
    target_link_libraries(opentrack-tracker-linux-joystick PRIVATE)
endif()
