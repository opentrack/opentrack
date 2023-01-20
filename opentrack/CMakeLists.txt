if(MSVC)
    add_compile_options(-EHsc)
    add_definitions(-D_HAS_EXCEPTIONS=1)
endif()

otr_module(executable EXECUTABLE BIN)

set_target_properties(opentrack-executable PROPERTIES
    SUFFIX "${opentrack-binary-suffix}"
    OUTPUT_NAME "opentrack"
    PREFIX ""
)

set_source_files_properties(resources.rc OBJECT_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/opentrack.ico")

target_link_libraries(${self} opentrack-user-interface opentrack-version)
