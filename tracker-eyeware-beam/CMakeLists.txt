# The Eyeware Beam SDK can be found at https://beam.eyeware.tech/developers/
# The latest version can be downloaded at https://eyewarecistorage.blob.core.windows.net/beam-sdk/BeamSDK-Windows64-1.1.0.zip
set(SDK_EYEWARE_BEAM "" CACHE PATH "Eyeware Beam SDK path")
if(WIN32 AND SDK_EYEWARE_BEAM)
    otr_module(tracker-eyeware-beam)

    target_include_directories(${self} SYSTEM PRIVATE "${SDK_EYEWARE_BEAM}/API/cpp/include")
    target_link_directories(${self} SYSTEM PRIVATE "${SDK_EYEWARE_BEAM}/API/cpp/lib")
    set(dll "${SDK_EYEWARE_BEAM}/API/cpp/lib/tracker_client.dll" "${SDK_EYEWARE_BEAM}/API/cpp/lib/libsodium.dll")
    set(lib tracker_client)

    message(${self})
    message(${dll})
    message(${lib})

    target_link_libraries(${self} ${lib})
    install(FILES ${dll} DESTINATION ${opentrack-libexec})
endif()
