if(WIN32)
    # https://www.nuget.org/packages/Tobii.StreamEngine.Native/
    set(SDK_TOBII "" CACHE PATH "Tobii Stream Engine path")
endif()
if(WIN32 AND SDK_TOBII)
    otr_module(tracker-tobii)

    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set(arch "x86")
    else()
        set(arch "x64")
    endif()

    target_include_directories(${self} SYSTEM PRIVATE "${SDK_TOBII}/include")
    target_link_directories(${self} PRIVATE "${SDK_TOBII}/lib/${arch}")

    set(dll "${SDK_TOBII}/lib/${arch}/tobii_stream_engine.dll")
    set(lib tobii_stream_engine.lib)

    target_link_libraries(${self} PRIVATE ${lib})
    install(FILES ${dll} DESTINATION ${opentrack-libexec})
endif()
