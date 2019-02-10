if(LINUX OR APPLE)
    set(SDK_XPLANE "" CACHE PATH "Path to the X-Plane SDK")

    if(SDK_XPLANE)
        otr_module(xplane-plugin NO-QT)
        # probably librt already included
        #install(FILES ${opentrack-xplane-plugin-c} DESTINATION "${opentrack-doc-src-pfx}/opentrack-xplane-plugin")
        target_include_directories(opentrack-xplane-plugin SYSTEM PUBLIC ${SDK_XPLANE}/CHeaders ${SDK_XPLANE}/CHeaders/XPLM)

        if(APPLE)
            target_compile_options(${self} PRIVATE
                                   -iframework "${SDK_XPLANE}/Libraries/Mac/"
                                   -DAPL -DXPLM200 -DXPLM210
                                   -framework XPLM -framework XPWidgets)
            target_link_options(${self}
                                "-F${SDK_XPLANE}/Libraries/Mac/"
                                -framework XPLM -framework XPWidgets)
        elseif(CMAKE_COMPILER_IS_GNUCXX)
            target_compile_options(${self} PRIVATE -DLIN -DXPLM200 -DXPLM210)
            target_link_options(${self} PRIVATE -rdynamic -nodefaultlibs)
        endif()

        if(CMAKE_COMPILER_IS_GNUCC AND NOT CMAKE_COMPILER_IS_CLANG)
            target_link_options(${self} PRIVATE -undefined_warning)
        endif()

        set_target_properties(${self} PROPERTIES
            LIBRARY_OUTPUT_NAME "opentrack.xpl"
            PREFIX "" SUFFIX "")

        if(UNIX AND NOT APPLE)
            target_link_libraries(${self} rt)
        endif()
    endif()
endif()
