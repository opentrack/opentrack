if(WIN32)
    SET(SDK_RIFT_140 "" CACHE PATH "libOVR 1.4.0 path for Oculus Rift")
    if(SDK_RIFT_140)
        include_directories("${SDK_RIFT_140}/Include")
        if(opentrack-64bit)
            link_directories("${SDK_RIFT_140}/Lib/Windows/x64/Release/VS2017")
        else()
            link_directories("${SDK_RIFT_140}/Lib/Windows/Win32/Release/VS2017")
        endif()
        include_directories("${SDK_RIFT_140}/build/Include") # for CI automation
        link_directories("${SDK_RIFT_140}/build/Release") # for CI automation
        link_libraries(LibOVR winmm setupapi ws2_32 imagehlp wbemuuid)

        otr_module(tracker-rift)
    endif()
endif()
