function(cleanup_build_dir)
    if(NOT $ENV{USERNAME} STREQUAL "sthalik")
        message(WARNING "you can't run this potentially destructive function")
        message(FATAL_ERROR "if you're sure, remove this line")
    endif()

    file(GLOB_RECURSE files LIST_DIRECTORIES TRUE RELATIVE "${CMAKE_BINARY_DIR}" "*")

    set(files_ "")

    set(got-install FALSE)
    set(got-install-file FALSE)
    set(got-cache FALSE)

    foreach(i ${files})
          if(i STREQUAL "CMakeCache.txt")
            set(got-cache TRUE)
            continue()
        endif()

        list(APPEND files_ "${CMAKE_BINARY_DIR}/${i}")
    endforeach()

    unset(files)

    if(NOT got-cache)
        message(FATAL_ERROR "")
    endif()

    # let's hope nothing bad happens
    file(REMOVE_RECURSE ${files_})

    execute_process(COMMAND cmake . WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" OUTPUT_QUIET)
endfunction()

cleanup_build_dir()

