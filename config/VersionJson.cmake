list(APPEND CMAKE_MODULE_PATH "${SRC_DIR}/config")
include(Version)

set(BOOTLOADER_INCLUDED "true")
set(BOOTLOADER_FILENAME "ecoboot.bin")
execute_process(
    COMMAND grep "release:" "${CMAKE_BINARY_DIR}/update/download_info.txt"
    COMMAND awk "{print $2}"
    OUTPUT_VARIABLE BOOTLOADER_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(
    ${SRC_DIR}/config/version.json.cmake_template
    ${CMAKE_BINARY_DIR}/update/version.json
    @ONLY
    )
