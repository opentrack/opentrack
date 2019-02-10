if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    otr_module(proto-iokit-foohid)
    target_link_options(${self} PRIVATE -framework IOKit)
endif()
