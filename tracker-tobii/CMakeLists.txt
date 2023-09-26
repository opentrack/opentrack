# https://developer.tobii.com/download-packages/tobii-xr-native-sdk/
set(SDK_TOBII "" CACHE PATH "Tobii Stream Engine path")
if(WIN32 AND SDK_TOBII)
    otr_module(tracker-tobii)

    target_include_directories(${self} SYSTEM PRIVATE "${SDK_TOBII}/include")
    target_link_directories(${self} PRIVATE "${SDK_TOBII}/lib/tobii")

    set(dll "${SDK_TOBII}/lib/tobii/tobii_stream_engine.dll")
    set(lib tobii_stream_engine.lib)

    target_link_libraries(${self} ${lib})
    install(FILES ${dll} DESTINATION ${opentrack-libexec})
endif()
