opentrack_boilerplate(opentrack-compat NO-COMPAT BIN)
if(NOT WIN32 AND NOT APPLE)
    target_link_libraries(opentrack-compat rt)
endif()
