IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    otr_module(proto-iokit-foohid)
    set_property(TARGET opentrack-proto-iokit-foohid APPEND_STRING PROPERTY LINK_FLAGS "-framework IOKit ")
endif()
