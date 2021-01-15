list(APPEND CMAKE_MODULE_PATH "${SRC_DIR}/config")
include(Version)

configure_file(
    ${SRC_DIR}/source/version.hpp.template
    ${CMAKE_BINARY_DIR}/source/version.hpp
    )

message("GIT_REV: ${GIT_REV}")
message("GIT_TAG: ${GIT_TAG}")
message("Version: ${CMAKE_PROJECT_VERSION}")
