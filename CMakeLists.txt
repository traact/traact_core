cmake_minimum_required(VERSION 3.16)

include(GenerateExportHeader)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# conan is required hard, library name, version and description are configured from conan
if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
else ()
    message(FATAL_ERROR "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()

option(WITH_TESTS "Build Tests" ON)

project(${CONAN_PACKAGE_NAME}_PROJECT VERSION ${CONAN_PACKAGE_VERSION})

conan_basic_setup(TARGETS)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_definitions(SPDLOG_DEBUG_ON SPDLOG_TRACE_ON SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # using Clang
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # using GCC
    # add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -pedantic -Wcast-align -Wpedantic -Wmisleading-indentation -Wlogical-op -Wnull-dereference)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    # using Intel C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # using Visual Studio C++
endif ()

set(CORE_SRC src/traact/traact.h
        src/traact/component/Component.h
        src/traact/component/ComponentTypes.h
        src/traact/datatypes.h
        src/traact/dataflow/Network.h
        src/traact/pattern/Pattern.h
        src/traact/pattern/Port.h
        src/traact/pattern/CoordinateSystem.h
        src/traact/util/FileUtil.h
        src/traact/util/FileUtil.cpp
        src/traact/pattern/MetaPattern.h
        src/traact/component/ComponentGraph.h
        src/traact/buffer/ComponentBuffer.h
        src/traact/buffer/TimeDomainBuffer.h
        src/traact/buffer/TimeDomainBuffer.cpp
        src/traact/pattern/instance/PatternInstance.h
        src/traact/pattern/instance/PortInstance.h
        src/traact/pattern/instance/GraphInstance.h
        src/traact/util/Utils.h
        src/traact/facade/Facade.cpp
        src/traact/facade/Facade.h
        src/traact/component/facade/ApplicationAsyncSource.h
        src/traact/component/facade/ApplicationSyncSink.h
        src/traact/component/PlayerBaseComponent.cpp
        src/traact/component/PlayerBaseComponent.h

        src/traact/component/FileWriter.cpp
        src/traact/component/FileWriter.h
        src/traact/component/FileReader.cpp
        src/traact/component/FileReader.h
        src/traact/component/FilePlayer.cpp
        src/traact/component/FilePlayer.h
        src/traact/component/FileRecorder.h
        src/traact/facade/PluginFactory.h
        src/traact/serialization/JsonPattern.h
        src/traact/serialization/JsonPatternInstance.h
        src/traact/serialization/JsonPort.h
        src/traact/serialization/JsonPortInstance.h
        src/traact/util/Logging.cpp
        src/traact/util/Logging.h
        src/traact/facade/rttr/RTTRPluginFactory.h
        src/traact/facade/rttr/RTTRPluginFactory.cpp
        src/traact/serialization/JsonGraphInstance.h
        src/traact/pattern/ParameterUtils.h
        src/traact/component/ModuleComponent.h
        src/traact/buffer/TimeStepBuffer.h
        src/traact/buffer/TimeStepBuffer.cpp
        src/traact/pattern/Pattern.cpp
        src/traact/pattern/Port.cpp
        src/traact/pattern/CoordinateSystem.cpp
        src/traact/pattern/MetaPattern.cpp
        src/traact/pattern/instance/PatternInstance.cpp
        src/traact/pattern/instance/PortInstance.cpp
        src/traact/pattern/instance/GraphInstance.cpp
        src/traact/util/Utils.cpp
        src/traact/dataflow/Network.cpp
        src/traact/pattern/ParameterUtils.cpp
        src/traact/component/Component.cpp
        src/traact/component/ModuleComponent.cpp
        src/traact/component/ComponentGraph.cpp
        src/traact/util/Semaphore.h
        src/traact/facade/DefaultFacade.cpp
        src/traact/facade/DefaultFacade.h
        src/traact/serialization/JsonGraphInstance.cpp
        src/traact/serialization/JsonPortInstance.cpp
        src/traact/serialization/JsonPattern.cpp
        src/traact/serialization/JsonPatternInstance.cpp
        src/traact/serialization/JsonPort.cpp
        src/traact/serialization/JsonTimeDomainManagerConfig.cpp
        src/traact/dataflow/taskflow/TaskFlowNetwork.cpp
        src/traact/dataflow/taskflow/TaskFlowNetwork.h
        src/traact/dataflow/taskflow/TaskFlowGraph.cpp
        src/traact/dataflow/taskflow/TaskFlowGraph.h
        src/traact/buffer/ComponentBuffer.cpp
        src/traact/buffer/SourceComponentBuffer.cpp
        src/traact/buffer/SourceComponentBuffer.h
        src/traact/dataflow/taskflow/TaskFlowTaskFunctions.h
        src/traact/dataflow/taskflow/TaskFlowTimeDomain.cpp
        src/traact/dataflow/taskflow/TaskFlowTimeDomain.h
        src/traact/buffer/DataFactory.h src/traact/buffer/BufferUtils.h src/traact/buffer/DataBuffer.cpp src/traact/buffer/DataBuffer.h src/traact/buffer/DataBufferFactory.h src/traact/dataflow/taskflow/TaskFlowUtils.h src/traact/util/EnumToString.cpp src/traact/util/EnumToString.h)


add_library(${CONAN_PACKAGE_NAME} SHARED
        ${CORE_SRC}
        )

conan_target_link_libraries(${CONAN_PACKAGE_NAME})

set_target_properties(${CONAN_PACKAGE_NAME} PROPERTIES VERSION ${CONAN_PACKAGE_VERSION})

target_include_directories(${CONAN_PACKAGE_NAME} PUBLIC
        ${CONAN_INCLUDE_DIRS_NLOHMANN_JSON}
        )

target_include_directories(${CONAN_PACKAGE_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_BINARY_DIR}
        )

# public header
generate_export_header(${CONAN_PACKAGE_NAME} EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/traact/${CONAN_PACKAGE_NAME}_export.h)

install(TARGETS ${CONAN_PACKAGE_NAME}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})


install(DIRECTORY "./src/traact" DESTINATION "${CMAKE_INSTALL_PREFIX}/include" FILES_MATCHING
        PATTERN "*.h"
        PATTERN "_internal" EXCLUDE
        )

add_subdirectory(apps)

if (WITH_TESTS)
    add_subdirectory(tests)
endif ()

