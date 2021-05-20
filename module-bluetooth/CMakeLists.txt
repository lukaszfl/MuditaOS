# Copyright (c) 2017-2021, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

project(module-bluetooth VERSION 1.0 DESCRIPTION "Bluetooth module library")

set(CMAKE_CXX_STANDARD 17)

module_is_test_entity()

set(SOURCES
    Bluetooth/audio/BluetoothAudioDevice.cpp
    Bluetooth/BluetoothWorker.cpp
    Bluetooth/WorkerController.cpp
    Bluetooth/CommandHandler.cpp
    Bluetooth/glucode/BluetoothRunLoop.cpp
    Bluetooth/interface/BluetoothDriverImpl.cpp
    Bluetooth/BtKeysStorage.cpp
    Bluetooth/interface/profiles/GAP/GAP.cpp
    Bluetooth/interface/profiles/A2DP/A2DP.cpp
    Bluetooth/interface/profiles/A2DP/AVRCP.cpp
    Bluetooth/interface/profiles/A2DP/AVDTP.cpp
    Bluetooth/interface/profiles/HSP/HSP.cpp
    Bluetooth/interface/profiles/HFP/HFP.cpp
    Bluetooth/interface/profiles/SCO.cpp
    Bluetooth/interface/profiles/Profile.cpp
    Bluetooth/interface/profiles/ProfileManager.cpp
    Bluetooth/interface/profiles/CellularInterface.cpp
    )

message("Build with BlueKitchen")
include(lib/btstack.cmake)

message(${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
message("Sources: " ${SOURCES})
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${BOARD_DIR_SOURCES})


# # Board specific compilation definitions,options,include directories and features
target_compile_definitions(${PROJECT_NAME} PUBLIC   ${PROJECT_CONFIG_DEFINITIONS}
                                                    ${PROJECT_TARGET}
                                                    ${TARGET_COMPILE_DEFINITIONS}
                                                    ${BOARD_DIR_DEFINITIONS}
                                                    )
target_compile_features(${PROJECT_NAME} PUBLIC ${TARGET_COMPILE_FEATURES})
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
)
target_link_options(${PROJECT_NAME} PUBLIC ${TARGET_LINK_OPTIONS})

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    ${BOARD_DIR_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${TARGET_LIBRARIES_INCLUDES}
    ${BT_STACK_ROOT}/src
    ${BT_STACK_ROOT}/src/classic


)

target_link_libraries(${PROJECT_NAME}
    service-evtmgr
    module-bsp
    service-bluetooth
    module-sys
    module-audio
    service-audio
    ${BOARD_DIR_LIBRARIES}
    )

if (${ENABLE_TESTS})
    add_subdirectory(tests)
endif()
