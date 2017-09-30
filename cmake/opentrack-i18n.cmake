function(otr_i18n_for_target_directory n)
    set(k "opentrack-${n}")

    get_property(lupdate-binary TARGET "${Qt5_LUPDATE_EXECUTABLE}" PROPERTY IMPORTED_LOCATION)

    foreach(i ${opentrack_all-translations})
        set(t "${CMAKE_CURRENT_SOURCE_DIR}/lang/${i}.ts")
        set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY CLEAN_NO_CUSTOM 1)
        if(NOT opentrack_disable-i18n-update)
            add_custom_command(OUTPUT "${t}"
                COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/lang"
                COMMAND "${lupdate-binary}" -silent -recursive -no-obsolete -locations relative . -ts "${t}"
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                DEPENDS ${${k}-cc} ${${k}-hh} ${${k}-ui} ${${k}-rc}
                COMMENT "Running lupdate for ${n}/${i}")
            set(target-name "i18n-lang-${i}-module-${n}")
            add_custom_target(${target-name} DEPENDS "${t}")
        endif()
        set_property(GLOBAL APPEND PROPERTY "opentrack-ts-files-${i}" "${t}")
    endforeach()
endfunction()

function(otr_merge_translations)
    install(CODE "file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/i18n\")")

    set(all-qm-files "")

    foreach(i ${opentrack_all-translations})
        get_property(ts-files GLOBAL PROPERTY "opentrack-ts-files-${i}")

        set(ts-files_ "")

        foreach(k ${ts-files})
            if(EXISTS "${k}" OR NOT opentrack_disable-i18n-update)
                list(APPEND ts-files_ "${k}")
            endif()
        endforeach()

        get_property(lrelease-binary TARGET "${Qt5_LRELEASE_EXECUTABLE}" PROPERTY IMPORTED_LOCATION)

        if(NOT ".${ts-files_}" STREQUAL ".")
            set(qm-output "${CMAKE_CURRENT_BINARY_DIR}/${i}.qm")
            list(APPEND all-qm-files "${qm-output}")
            add_custom_command(OUTPUT "${qm-output}"
                COMMAND "${lrelease-binary}" -nounfinished -silent ${ts-files_} -qm "${qm-output}"
                DEPENDS ${ts-files}
                COMMENT "Running lrelease for ${i}")
            set(lang-target "i18n-lang-${i}")
            add_custom_target("${lang-target}" DEPENDS "${qm-output}")
            install(FILES "${qm-output}" DESTINATION "${opentrack-i18n-pfx}" RENAME "${i}.qm" ${opentrack-perms})
        endif()
    endforeach()
    add_custom_target(i18n ALL DEPENDS ${all-qm-files})
endfunction()
