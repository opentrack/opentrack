otr_module(executable EXECUTABLE BIN)

set_target_properties(opentrack-user-interface PROPERTIES
    SUFFIX "${opentrack-binary-suffix}"
    OUTPUT_NAME "opentrack"
)

target_link_libraries(opentrack-executable opentrack-user-interface)
