include_guard(GLOBAL)

if(POLICY CMP0177)
    cmake_policy(SET CMP0177 NEW)
endif()

add_custom_target(i18n-lupdate)
add_custom_target(i18n-lrelease DEPENDS i18n-lupdate)
add_custom_target(i18n ALL DEPENDS i18n-lrelease)

function(otr_i18n_for_target_directory n)
    set(k "opentrack-${n}")

    get_property(lupdate-binary TARGET Qt6::lupdate PROPERTY IMPORTED_LOCATION)

    #make_directory("${CMAKE_CURRENT_BINARY_DIR}/lang")

    set(ts-files "")
    foreach(k ${opentrack_all-translations})
        list(APPEND ts-files "${CMAKE_CURRENT_SOURCE_DIR}/lang/${k}.ts")
        set_property(GLOBAL APPEND PROPERTY "opentrack-ts-files-${k}" "${CMAKE_CURRENT_SOURCE_DIR}/lang/${k}.ts")
    endforeach()
    set(stamp "${CMAKE_CURRENT_BINARY_DIR}/lupdate.stamp")

    foreach(i ${opentrack_all-translations})
        set(t "${CMAKE_CURRENT_SOURCE_DIR}/lang/${i}.ts")
        set(input "${${k}-all}")
        if (NOT EXISTS "${t}")
            file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lang")
            file(READ "${CMAKE_SOURCE_DIR}/cmake/translation-stub.ts" stub)
            string(REPLACE "@LANG@" "${i}" stub "${stub}")
            file(WRITE "${t}" "${stub}")
        endif()
    endforeach()

    add_custom_command(OUTPUT "${stamp}"
                       COMMAND "${lupdate-binary}"
                       -I "${CMAKE_SOURCE_DIR}"
                       -silent
                       -recursive
                       -no-obsolete
                       -locations none
                       .
                       -ts ${ts-files}
                       COMMAND "${CMAKE_COMMAND}" -E touch "${stamp}"
                       #BYPRODUCTS ${ts-files}
                       DEPENDS "opentrack-${n}"
                       COMMENT "Running lupdate for ${n}"
                       WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
    set(target-name "i18n-module-${n}")
    add_custom_target(${target-name} DEPENDS "${stamp}" COMMENT "")
    add_dependencies(i18n-lupdate ${target-name})
endfunction()

function(otr_merge_translations)
    otr_escape_string(i18n-pfx "${opentrack-i18n}")
    #install(CODE "message(FATAL_ERROR \"foo \${CMAKE_INSTALL_PREFIX}\")")
    #install(CODE "file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/${i18n-pfx}\")")

    foreach(i ${opentrack_all-translations})
        get_property(ts-files GLOBAL PROPERTY "opentrack-ts-files-${i}")
        get_property(lrelease-binary TARGET Qt6::lrelease PROPERTY IMPORTED_LOCATION)

        set(qm-output "${CMAKE_BINARY_DIR}/${i}.qm")

        # whines about duplicate messages since tracker-pt-base is static
        if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
            set(to-null "2>NUL")
        else()
            set(to-null "2>/dev/null")
        endif()

        add_custom_command(OUTPUT "${qm-output}"
            COMMAND "${lrelease-binary}"
                -nounfinished
                -silent
                #-verbose
                ${ts-files}
                -qm "${qm-output}"
                ${to-null}
            DEPENDS i18n-lupdate ${ts-files}
            COMMENT "Running lrelease for ${i}"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

        set(target-name i18n-qm-${i})
        add_custom_target(${target-name} DEPENDS "${qm-output}")
        add_dependencies(i18n-lrelease ${target-name})

        install(FILES "${qm-output}"
                DESTINATION "${opentrack-i18n}"
                PERMISSIONS ${opentrack-perms-file})
    endforeach()
endfunction()
