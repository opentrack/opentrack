if(WIN32)
    otr_module(wiiyourself STATIC NO-COMPAT NO-QT)
    target_link_libraries(${self} setupapi hid winmm)
endif()
