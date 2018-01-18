otr_module(executable EXECUTABLE BIN)

set_target_properties(opentrack-executable PROPERTIES
    SUFFIX "${opentrack-binary-suffix}"
    OUTPUT_NAME "trackmouse"
    PREFIX ""
)

target_link_libraries(opentrack-executable opentrack-user-interface)
