if(WIN32)
    otr_module(dinput BIN)
    target_link_libraries(opentrack-dinput dinput8)
endif()
