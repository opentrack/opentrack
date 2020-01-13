if(APPLE)
    otr_escape_string(srcdir "${CMAKE_SOURCE_DIR}")
    otr_escape_string(instdir "${CMAKE_INSTALL_PREFIX}")
    otr_escape_string(commit "${OPENTRACK_COMMIT}")
    install(CODE "
       execute_process(COMMAND /bin/sh \"${srcdir}/macosx/make-app-bundle.sh\"
                                        \"${srcdir}/macosx\"
                                        \"${instdir}\"
                                       \"${commit}\")
    ")
endif()
