if(WIN32 AND Qt5SerialPort_FOUND)
    otr_module(tracker-edtracker)
    target_link_libraries(${self} opentrack-dinput ${Qt5SerialPort_LIBRARIES})
    target_include_directories(${self} SYSTEM PUBLIC ${Qt5SerialPort_INCLUDE_DIRS})
endif()

