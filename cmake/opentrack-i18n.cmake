include_guard(GLOBAL)

add_custom_target(i18n ALL)
add_custom_target(i18n-lupdate)
add_custom_target(i18n-lrelease)
add_dependencies(i18n-lrelease i18n-lupdate)
add_dependencies(i18n i18n-lrelease)

function(otr_i18n_for_target_directory n)
    set(k "opentrack-${n}")

    get_property(lupdate-binary TARGET "${Qt5_LUPDATE_EXECUTABLE}" PROPERTY IMPORTED_LOCATION)
    set(target-names "")

    #make_directory("${CMAKE_CURRENT_BINARY_DIR}/lang")

    foreach(i ${opentrack_all-translations})
        set(t "${CMAKE_CURRENT_SOURCE_DIR}/lang/${i}.ts")
        set(t2 "${i}.ts")
        set(input "${${k}-all}")
        if (NOT EXISTS "${t}")
            file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lang")
            file(READ "${CMAKE_SOURCE_DIR}/cmake/translation-stub.ts" stub)
            file(WRITE "${t}" "${stub}")
        endif()
        add_custom_command(OUTPUT "${t2}"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/lang"
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/lang"
            COMMAND "${lupdate-binary}"
                -I "${CMAKE_SOURCE_DIR}"
                -silent
                -recursive
                -no-obsolete
                -locations none
                .
                -ts "${t}"
            COMMAND "${CMAKE_COMMAND}" -E copy "${t}" "${CMAKE_CURRENT_BINARY_DIR}/${t2}"
            DEPENDS "${k}" "${t}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
            COMMENT "Running lupdate for ${n}/${i}")
        set(target-name "i18n-lang-${i}-ts-${n}")
        list(APPEND target-names "${target-name}")
        add_custom_target(${target-name} DEPENDS "${t2}" COMMENT "")
        set_property(GLOBAL APPEND PROPERTY "opentrack-ts-files-${i}" "${CMAKE_CURRENT_BINARY_DIR}/${t2}")
        set_property(GLOBAL APPEND PROPERTY "opentrack-ts-module-${n}" "${target-name}")
        add_dependencies(i18n-lupdate "${target-name}")
    endforeach()
endfunction()

function(otr_merge_translations)
    otr_escape_string(i18n-pfx "${opentrack-i18n-pfx}")
    install(CODE "file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/${i18n-pfx}\")")

    set(all-ts-targets "")

    get_property(all-modules GLOBAL PROPERTY opentrack-all-modules)
    foreach(target ${all-modules})
        get_property(ts-targets GLOBAL PROPERTY "opentrack-ts-module-${target}")
        list(APPEND all-ts-targets "${ts-targets}")
    endforeach()

    foreach(i ${opentrack_all-translations})
        get_property(ts-files GLOBAL PROPERTY "opentrack-ts-files-${i}")
        get_property(lrelease-binary TARGET "${Qt5_LRELEASE_EXECUTABLE}" PROPERTY IMPORTED_LOCATION)

        set(qm-output "${i}.qm")

        # whines about duplicate messages since tracker-pt-base is static
        if(WIN32)
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
            DEPENDS ${all-ts-targets}
            COMMENT "Running lrelease for ${i}"
            WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

        set(target-name  "i18n-qm-${i}")
        add_custom_target("${target-name}" DEPENDS "${qm-output}")
        add_dependencies(i18n-lrelease "${target-name}")

        install(FILES "${CMAKE_BINARY_DIR}/${qm-output}"
                DESTINATION "${CMAKE_INSTALL_PREFIX}/${opentrack-i18n-pfx}"
                PERMISSIONS ${opentrack-perms-file})
    endforeach()
endfunction()
