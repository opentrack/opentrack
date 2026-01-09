otr_module(tracker-xreal-one)

include(FetchContent)
FetchContent_Declare(
  AHRSFusion
  GIT_REPOSITORY https://github.com/xioTechnologies/Fusion.git
  GIT_TAG        v1.2.11
  SOURCE_SUBDIR  Fusion
)

FetchContent_MakeAvailable(AHRSFusion)

set_target_properties(Fusion PROPERTIES
  PROJECT_LABEL ${self}-fusion
  FOLDER "${self}/dependencies"
  LINKER_LANGUAGE C
)

target_link_libraries(${self} PRIVATE Fusion)