include_guard(GLOBAL)

function(otr_dist_select_variant)
    file(GLOB variants "variant/*")

    set(variant-list "")

    foreach(k ${variants})
        get_filename_component(name "${k}" NAME)
        if(EXISTS "${k}/_variant.cmake" AND EXISTS "${k}/CMakeLists.txt")
            list(APPEND variant-list "${name}")
        else()
            message(FATAL_ERROR "Stray item in variant dir '${name}'")
        endif()
    endforeach()

    set(opentrack_variant "default" CACHE STRING "")
    set(dir "${CMAKE_SOURCE_DIR}/variant/${opentrack_variant}")

    if(NOT EXISTS "${dir}/_variant.cmake" OR NOT EXISTS "${dir}/CMakeLists.txt")
        set(opentrack_variant "default" CACHE STRING "" FORCE)
        set(dir "${CMAKE_SOURCE_DIR}/variant/${opentrack_variant}")
    endif()

    set_property(CACHE opentrack_variant PROPERTY STRINGS "${variant-list}")

    include("${dir}/_variant.cmake")
    otr_init_variant()

    add_subdirectory("${dir}")
endfunction()
