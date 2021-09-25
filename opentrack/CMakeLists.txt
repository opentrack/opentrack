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

target_link_libraries(${self} opentrack-user-interface opentrack-version)
