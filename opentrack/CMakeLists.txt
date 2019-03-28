otr_module(executable EXECUTABLE BIN)

set_target_properties(opentrack-executable PROPERTIES
    SUFFIX "${opentrack-binary-suffix}"
    OUTPUT_NAME "opentrack"
    PREFIX ""
)

target_link_libraries(${self} opentrack-user-interface opentrack-version)
