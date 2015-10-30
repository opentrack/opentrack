if(WIN32)
    opentrack_boilerplate(opentrack-proto-freetrack)
    target_link_libraries(opentrack-proto-freetrack opentrack-csv opentrack-compat)
endif()
