add_library(sys-manager STATIC)

target_sources(sys-manager
    PUBLIC
        include/SystemManager/DependencyGraph.hpp
        include/SystemManager/CpuSentinel.hpp
        include/SystemManager/CpuStatistics.hpp
        include/SystemManager/SystemManagerCommon.hpp
        include/SystemManager/CpuGovernor.hpp
        include/SystemManager/PowerManager.hpp
        include/SystemManager/DeviceManager.hpp
    
    PRIVATE
        CpuGovernor.cpp
        CpuSentinel.cpp
        CpuStatistics.cpp
        data/SystemManagerActionsParams.hpp
        DependencyGraph.cpp
        DeviceManager.cpp
        graph/TopologicalSort.cpp
        graph/TopologicalSort.hpp
        PowerManager.cpp
        SystemManagerCommon.cpp
)

target_include_directories(sys-manager
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
)

target_link_libraries(sys-manager
    PUBLIC
        apps-common
        service-evtmgr
        eventstore
        sys-service
        sys-common
    PRIVATE
        service-desktop
)

if (${ENABLE_TESTS})
    add_subdirectory(tests)
endif()
