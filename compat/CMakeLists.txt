otr_module(compat NO-COMPAT BIN)

if(NOT WIN32 AND NOT APPLE)
    target_link_libraries(${self} PRIVATE rt)
endif()

if(WIN32)
    target_link_libraries(${self} PRIVATE strmiids)
endif()


if(APPLE)
    target_link_libraries(${self} PRIVATE proc)
    # AVFoundation-based camera enumeration: matches what OpenCV's
    # CAP_AVFOUNDATION backend sees, eliminating the index mismatch
    # that QMediaDevices-based enumeration produced after USB hot-plug.
    target_sources(${self} PRIVATE camera-names-apple.mm)
    set_source_files_properties(camera-names-apple.mm PROPERTIES
        COMPILE_FLAGS "-fobjc-arc")
    target_sources(${self} PRIVATE appnap_mac.mm)
    set_source_files_properties(appnap_mac.mm PROPERTIES
        COMPILE_FLAGS "-fno-objc-arc")
    target_sources(${self} PRIVATE background-app_mac.mm)
    set_source_files_properties(background-app_mac.mm PROPERTIES
        COMPILE_FLAGS "-fobjc-arc")
    target_link_libraries(${self} PRIVATE
        "-framework Foundation"
        "-framework AppKit"
        "-framework AVFoundation")
elseif(LINUX)
    otr_pkgconfig_(has-libproc2 ${self} libproc2)
    if(has-libproc2)
        target_compile_definitions(${self} PUBLIC -DOTR_HAS_LIBPROC2)
    else()
        otr_pkgconfig_(has-libprocps ${self} libprocps)
        if(NOT has-libprocps)
            message(FATAL_ERROR "install libproc2 or libprocps development files")
        endif()
    endif()
endif()
