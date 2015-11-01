if(APPLE)
    install(CODE "
        execute_process(COMMAND /bin/sh \"${CMAKE_SOURCE_DIR}/macosx/make-app-bundle.sh\"
                                        \"${CMAKE_SOURCE_DIR}/macosx\"
                                        \"${CMAKE_INSTALL_PREFIX}\"
                                        \"${CMAKE_BINARY_DIR}\"
                                        \"${OPENTRACK_COMMIT}\")
    ")
endif()
