cmake_minimum_required(VERSION 3.16)

include(GenerateExportHeader)

option(WITH_TESTS "Build Tests" ON)
option(WITH_COVERAGE "" ON)
option(WITH_COZ "With Coz Profiler" OFF)

set(CMAKE_CXX_STANDARD 17)

# public header
set(LIB_HEADERS
        include/traact/traact.h
        include/traact/traact_core_export.h
        include/traact/component/Component.h
        include/traact/component/ComponentTypes.h
        include/traact/datatypes.h
        include/traact/dataflow/Network.h
        include/traact/pattern/Pattern.h
        include/traact/pattern/Port.h
        include/traact/pattern/CoordinateSystem.h

        include/traact/pattern/MetaPattern.h
        include/traact/component/ComponentGraph.h
        include/traact/util/PerformanceMonitor.h
        include/traact/buffer/ComponentBuffer.h
        include/traact/buffer/TimeDomainBuffer.h
        include/traact/buffer/TimeDomainBuffer.cpp
        include/traact/buffer/BufferTypeConversion.h
        include/traact/buffer/BufferFactory.h
        include/traact/pattern/instance/PatternInstance.h
        include/traact/pattern/instance/PortInstance.h
        include/traact/pattern/instance/GraphInstance.h
        include/traact/util/Utils.h
        include/traact/facade/Facade.cpp
        include/traact/facade/Facade.h
        include/traact/component/facade/ApplicationAsyncSource.h
        include/traact/component/facade/ApplicationSyncSink.h
        include/traact/component/PlayerBaseComponent.cpp
        include/traact/component/PlayerBaseComponent.h

        include/traact/component/FileWriter.cpp
        include/traact/component/FileWriter.h
        include/traact/component/FileReader.cpp
        include/traact/component/FileReader.h
        include/traact/component/FilePlayer.cpp
        include/traact/component/FilePlayer.h
        include/traact/component/FileRecorder.h
        include/traact/facade/PluginFactory.h
        include/traact/serialization/JsonPattern.h
        include/traact/serialization/JsonPatternInstance.h
        include/traact/serialization/JsonPort.h
        include/traact/serialization/JsonPortInstance.h

        include/traact/util/FileUtil.h
        include/traact/util/Logging.cpp
        include/traact/util/Logging.h
        include/traact/facade/rttr/RTTRPluginFactory.cpp
        include/traact/facade/rttr/RTTRPluginFactory.h
        include/traact/serialization/JsonGraphInstance.h
        include/traact/pattern/ParameterUtils.h
        include/traact/component/ModuleComponent.h
        include/traact/util/TraactCoreUtils.h
        include/traact/util/TraactCoreUtils.cpp
        include/traact/buffer/TimeStepBuffer.h
        include/traact/buffer/TimeStepBuffer.cpp)

# rest of source code
set(LIB_SOURCES
        src/pattern/Pattern.cpp
        src/pattern/Port.cpp
        src/pattern/CoordinateSystem.cpp

        src/pattern/MetaPattern.cpp

        src/util/PerformanceMonitor.cpp
        src/pattern/instance/PatternInstance.cpp
        src/pattern/instance/PortInstance.cpp
        src/pattern/instance/GraphInstance.cpp
        src/util/Utils.cpp
        src/dataflow/Network.cpp
        src/pattern/ParameterUtils.cpp
        src/component/Component.cpp
        src/component/ModuleComponent.cpp
        src/component/ComponentGraph.cpp
        include/traact/util/Semaphore.h
        include/traact/buffer/BufferSource.h
        include/traact/facade/DefaultFacade.cpp
        include/traact/facade/DefaultFacade.h
        include/traact/util/FileUtil.cpp
        src/serialization/JsonGraphInstance.cpp
        src/serialization/JsonPortInstance.cpp
        src/serialization/JsonPattern.cpp
        src/serialization/JsonPatternInstance.cpp
        src/serialization/JsonPort.cpp
        src/serialization/JsonTimeDomainManagerConfig.cpp
        include/traact/util/RingBuffer.h
        include/traact/dataflow/taskflow/TaskFlowNetwork.cpp
        include/traact/dataflow/taskflow/TaskFlowNetwork.h
        include/traact/dataflow/taskflow/TaskFlowGraph.cpp
        include/traact/dataflow/taskflow/TaskFlowGraph.h
        include/traact/buffer/ComponentBuffer.cpp
        include/traact/buffer/SourceComponentBuffer.cpp
        include/traact/buffer/SourceComponentBuffer.h include/traact/dataflow/taskflow/TaskFlowTaskFunctions.h include/traact/dataflow/taskflow/TaskFlowTimeDomain.cpp include/traact/dataflow/taskflow/TaskFlowTimeDomain.h)


# conan is required hard, library name, version and description are configured from conan
if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
else ()
    message(FATAL_ERROR "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()

project(${CONAN_PACKAGE_NAME}_PROJECT VERSION ${CONAN_PACKAGE_VERSION})

conan_basic_setup(TARGETS)

if (WITH_COVERAGE)
    #add_compile_options(-g -O0 -Wall -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage)
    add_link_options(-fprofile-arcs -ftest-coverage)
endif ()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DTBB_USE_DEBUG=1)
    add_compile_definitions(SPDLOG_DEBUG_ON SPDLOG_TRACE_ON SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)

endif ()

add_definitions(-DTBB_PREVIEW_FLOW_GRAPH_FEATURES=1)
add_definitions(-DTBB_PREVIEW_FLOW_GRAPH_NODES=1)

# conan workspace and cmake don't work that well together, did not manage to get the build dir into the includes: 
# force generated file into source include dir 
# only one export file could be generated
# use files and defines instead
#set(CMAKE_CXX_VISIBILITY_PRESET hidden)
#set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)
#generate_export_header(${CONAN_PACKAGE_NAME} EXPORT_FILE_NAME "../../include/traact/${CONAN_PACKAGE_NAME}_export.h")

if(UNIX)
    if(APPLE)
        MESSAGE(STATUS "Building for Macos.")
        add_definitions(-DTRAACT_TARGET_APPLE=1)
    endif()
    MESSAGE(STATUS "Building for Unix.")
    add_definitions(-DTRAACT_TARGET_UNIX=1)
elseif(WIN32)
    MESSAGE(STATUS "Building for Windows.")
    add_definitions(-DTRAACT_TARGET_WINDOWS=1)
endif()

if (MSVC)
    # per default disable extended aligned storage for now on msvc
    add_definitions(-D_DISABLE_EXTENDED_ALIGNED_STORAGE -DHAVE_SNPRINTF)
endif()


add_library(${CONAN_PACKAGE_NAME} SHARED
        ${LIB_HEADERS}
        ${LIB_SOURCES}
        )

if(WITH_COZ)
    find_package(coz-profiler)
endif()

conan_target_link_libraries(${CONAN_PACKAGE_NAME})

set_target_properties(${CONAN_PACKAGE_NAME} PROPERTIES VERSION ${CONAN_PACKAGE_VERSION})
#set_target_properties(${CONAN_PACKAGE_NAME} PROPERTIES PUBLIC_HEADER "${LIB_HEADERS}")


target_include_directories(${CONAN_PACKAGE_NAME} PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        ${CONAN_INCLUDE_DIRS_NLOHMANN_JSON}
        )
target_include_directories(${CONAN_PACKAGE_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

install(TARGETS ${CONAN_PACKAGE_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})


install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/traact" # source directory
        DESTINATION "include" # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.h" # select header files
        )

#install(FILES "${PROJECT_BINARY_DIR}/traact_export.h" DESTINATION "include")

add_subdirectory(apps)

if (WITH_TESTS)
    add_subdirectory(tests)
endif ()

