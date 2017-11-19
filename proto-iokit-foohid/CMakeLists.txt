IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    otr_module(proto-iokit-foohid)
    otr_prop(TARGET opentrack-proto-iokit-foohid LINK_FLAGS "-framework IOKit")
endif()
