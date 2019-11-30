if(APPLE)
    otr_module(proto-iokit-foohid)
    target_link_options(${self} PRIVATE -framework IOKit)
endif()
