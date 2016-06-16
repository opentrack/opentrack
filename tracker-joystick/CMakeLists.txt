if(WIN32)
    opentrack_boilerplate(opentrack-tracker-joystick)
    target_link_libraries(opentrack-tracker-joystick opentrack-logic)
endif()

