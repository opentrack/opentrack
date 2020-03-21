# https://developer.tobii.com/download-packages/stream-engine-4-1-0-for-windows-x86
# https://developer.tobii.com/download-packages/stream-engine-4-1-0-for-windows-x64
set(SDK_TOBII "" CACHE PATH "Tobii SDK path")
if(SDK_TOBII)
    otr_module(tracker-tobii)
    target_include_directories(${self} SYSTEM PRIVATE "${SDK_TOBII}/include")

    set(dll "${SDK_TOBII}/lib/tobii/tobii_stream_engine.dll")
    set(lib "${SDK_TOBII}/lib/tobii/tobii_stream_engine.lib")

    message(${self})
    message(${dll})
    message(${lib})

    install(FILES ${dll} DESTINATION ${opentrack-hier-pfx})
    target_link_libraries(${self} ${lib})
endif()
