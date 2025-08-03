if(WIN32)
    otr_module(proto-freetrack)
    target_link_libraries(opentrack-proto-freetrack PRIVATE opentrack-csv)
endif()
