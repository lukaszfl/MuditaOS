target_sources(
        platform
    PRIVATE
        src/LinuxPlatform.cpp
        src/BlockDeviceFactory.cpp
        src/DiskImage.cpp
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
