if(WIN32)
    otr_module(dinput BIN)
    target_link_libraries(opentrack-dinput PRIVATE dinput8 dxguid)
endif()
