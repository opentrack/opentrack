include_guard(GLOBAL)
include(FindPkgConfig)

function(otr_pkgconfig target)
    foreach(i ${ARGN})
        set(k pkg-config_${i})
        pkg_check_modules(${k} QUIET ${i})
        if(${${k}_FOUND})
            target_compile_options(${target} PRIVATE "${${k}_CFLAGS}")
            target_link_options(${target} PRIVATE ${${k}_LDFLAGS})
            target_include_directories(${target} SYSTEM PRIVATE ${${k}_INCLUDE_DIRS} ${${k}_INCLUDEDIR})
            target_link_libraries(${target} ${${k}_LIBRARIES})
        else()
            message(FATAL_ERROR "Can't find '${i}'. Please install development files for this package.")
        endif()
    endforeach()
endfunction()


