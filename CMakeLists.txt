cmake_minimum_required(VERSION 3.9)

include(GenerateExportHeader)

option(WITH_TESTS "Build Tests" ON)
option(WITH_COVERAGE "" ON)

set(CMAKE_CXX_STANDARD 17)

# public header
set(LIB_HEADERS
        include/traact/traact.h
        include/traact/traact_export.h
        include/traact/component/Component.h
        include/traact/component/ComponentTypes.h
        include/traact/datatypes.h
        include/traact/dataflow/Network.h
        include/traact/pattern/Pattern.h
        include/traact/pattern/Port.h
        include/traact/pattern/spatial/SpatialPattern.h
        include/traact/pattern/spatial/CoordinateSystem.h
        include/traact/buffer/TimeDomainManager.h
        include/traact/pattern/MetaPattern.h
        include/traact/component/ComponentGraph.h
        include/traact/util/PerformanceMonitor.h
        include/traact/buffer/GenericComponentBuffer.h
        include/traact/buffer/GenericTimeDomainBuffer.h
        include/traact/buffer/GenericBufferTypeConversion.h
        include/traact/buffer/GenericFactoryObject.h
        include/traact/pattern/instance/PatternInstance.h
        include/traact/pattern/instance/PortInstance.h
        include/traact/pattern/instance/GraphInstance.h
        include/traact/util/Utils.h
        include/traact/serialization/JsonPattern.h
        include/traact/serialization/JsonPatternInstance.h
        include/traact/serialization/JsonPort.h
        include/traact/serialization/JsonPortInstance.h
        include/traact/serialization/JsonSpatialPattern.h
        include/traact/serialization/JsonGraphInstance.h include/traact/pattern/ParameterUtils.h include/traact/component/ModuleComponent.h)

# rest of source code
set(LIB_SOURCES
        src/pattern/Pattern.cpp
        src/pattern/Port.cpp
        src/pattern/spatial/SpatialPattern.cpp
        src/pattern/spatial/CoordinateSystem.cpp
        src/buffer/TimeDomainManager.cpp
        src/dataflow/intern/DynamicJoinNode.h
        src/dataflow/intern/NetworkGraph.h
        src/dataflow/intern/TraactComponentBase.h
        src/dataflow/intern/TraactComponentFunctional.h
        src/dataflow/intern/TraactComponentSource.h
        src/dataflow/intern/TraactComponentSink.h
        src/pattern/MetaPattern.cpp
        src/buffer/GenericTimeDomainBuffer.cpp
        src/util/PerformanceMonitor.cpp
        src/dataflow/intern/TraactComponentFunctional.cpp
        src/dataflow/intern/TraactComponentSink.cpp
        src/dataflow/intern/TraactComponentSource.cpp
        src/dataflow/intern/NetworkGraph.cpp
        src/pattern/instance/PatternInstance.cpp
        src/pattern/instance/PortInstance.cpp
        src/pattern/instance/GraphInstance.cpp
        src/util/Utils.cpp
        src/dataflow/Network.cpp src/pattern/ParameterUtils.cpp src/component/Component.cpp src/component/ModuleComponent.cpp src/component/ComponentGraph.cpp)


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

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    add_definitions(-DTBB_USE_DEBUG=1)
endif ()

add_definitions(-DTBB_PREVIEW_FLOW_GRAPH_FEATURES=1)
add_definitions(-DTBB_PREVIEW_FLOW_GRAPH_NODES=1)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_library(${CONAN_PACKAGE_NAME} SHARED
        ${LIB_HEADERS}
        ${LIB_SOURCES}
        )

# conan workspace and cmake don't work that well together
generate_export_header(${CONAN_PACKAGE_NAME} BASE_NAME traact EXPORT_FILE_NAME "../../include/traact/traact_export.h")

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


if (WITH_TESTS)
    add_subdirectory(tests)
endif ()

