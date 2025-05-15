otr_module(compat NO-COMPAT BIN)

if(NOT WIN32 AND NOT APPLE)
    target_link_libraries(${self} rt)
endif()

if(WIN32)
    target_link_libraries(${self} strmiids)
endif()
