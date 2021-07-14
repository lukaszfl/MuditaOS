target_sources(
        platform
    PRIVATE
        src/RT1051Platform.cpp
        src/BlockDeviceFactory.cpp
	src/disk_eeprom.cpp
	src/disk_emmc.cpp
)

target_include_directories(
        platform
    PUBLIC
        include
)

target_link_libraries(
        platform
    PRIVATE
        module-vfs
)
