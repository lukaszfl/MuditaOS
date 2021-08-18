project(service-bluetooth)
message( "${PROJECT_NAME}  ${CMAKE_CURRENT_LIST_DIR}" )


set(SOURCES
    ServiceBluetooth.cpp
    service-bluetooth/SettingsHolder.cpp
    service-bluetooth/SettingsSerializer.cpp
    service-bluetooth/BluetoothMessage.cpp
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})


target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
)
target_compile_options(${PROJECT_NAME}
        PUBLIC
        ${TARGET_COMPILE_OPTIONS}
        PRIVATE
        -Wno-sign-compare
        -Wno-missing-field-initializers
        -Wno-unused-function
        -Wno-implicit-fallthrough

        # C only flags
        "$<$<COMPILE_LANGUAGE:C>:-Wno-old-style-declaration>"
        -O0
        -g3
        -ggdb
        )
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        module-bluetooth
        service-desktop
        module-sys
        json::json
)
