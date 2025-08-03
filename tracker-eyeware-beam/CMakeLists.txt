# The Eyeware Beam SDK can be found at https://beam.eyeware.tech/developers/
# The latest version can be downloaded at https://eyewarecistorage.blob.core.windows.net/beam-sdk/BeamSDK-Windows64-1.1.0.zip
set(SDK_EYEWARE_BEAM "" CACHE PATH "Eyeware Beam SDK path")
if(WIN32 AND SDK_EYEWARE_BEAM)
    if(MSVC)
        add_compile_options(-EHsc)
    endif()
    otr_module(tracker-eyeware-beam)

    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
        set(arch "x86")
    else()
        set(arch "x64")
    endif()

    target_include_directories(${self} SYSTEM PRIVATE "${SDK_EYEWARE_BEAM}/API/cpp/include")
    target_link_directories(${self} PRIVATE "${SDK_EYEWARE_BEAM}/API/cpp/lib/${arch}")
    set(dll "${SDK_EYEWARE_BEAM}/API/cpp/lib/${arch}/tracker_client.dll")
    set(lib tracker_client.lib)

    #message(${self})
    #message(${dll})
    #message(${lib})

    target_link_libraries(${self} PRIVATE ${lib})
    install(FILES ${dll} DESTINATION ${opentrack-libexec})
endif()
