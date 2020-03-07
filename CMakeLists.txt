cmake_minimum_required(VERSION 3.9)

option(WITH_TESTS "Build Tests" ON)
option(WITH_COVERAGE "" ON)

set(CMAKE_CXX_STANDARD 17)

# public header
set(LIB_HEADERS
        include/traact/traact.h
        include/traact/component/Component.h
        include/traact/component/ComponentTypes.h
        include/traact/datatypes.h
        include/traact/dataflow/TraactNetwork.h
        include/traact/pattern/Pattern.cpp
        include/traact/pattern/Pattern.h
        include/traact/pattern/Port.cpp
        include/traact/pattern/Port.h
        include/traact/pattern/spatial/SpatialPattern.cpp
        include/traact/pattern/spatial/SpatialPattern.h
        include/traact/pattern/spatial/CoordinateSystem.cpp
        include/traact/pattern/spatial/CoordinateSystem.h
        include/traact/buffer/TimeDomainManager.h
        include/traact/buffer/TimeDomainManager.cpp
        include/traact/dataflow/intern/DynamicJoinNode.h
        include/traact/dataflow/intern/NetworkGraph.h
        include/traact/dataflow/intern/TraactComponentBase.h
        include/traact/dataflow/intern/TraactComponentFunctional.h
        include/traact/dataflow/intern/TraactComponentSource.h
        include/traact/dataflow/intern/TraactComponentSink.h
        include/traact/pattern/MetaPattern.cpp
        include/traact/pattern/MetaPattern.h
        include/traact/component/ComponentGraph.cpp
        include/traact/component/ComponentGraph.h
        include/traact/util/PerformanceMonitor.h
        include/traact/util/PerformanceMonitor.cpp
        include/traact/facade/Facade.cpp
        include/traact/facade/Facade.h
        include/traact/facade/PluginFactory.cpp
        include/traact/facade/PluginFactory.h include/traact/buffer/GenericComponentBuffer.cpp include/traact/buffer/GenericComponentBuffer.h include/traact/buffer/GenericTimeDomainBuffer.cpp include/traact/buffer/GenericTimeDomainBuffer.h include/traact/buffer/GenericBufferTypeConversion.h include/traact/buffer/GenericFactoryObject.h include/traact/DoubleType.h include/traact/dataflow/intern/TraactComponentFunctional.cpp include/traact/dataflow/intern/TraactComponentSink.cpp include/traact/dataflow/intern/TraactComponentSource.cpp include/traact/dataflow/intern/NetworkGraph.cpp include/traact/facade/TraactPlugin.h include/traact/pattern/instance/PatternInstance.cpp include/traact/pattern/instance/PatternInstance.h include/traact/pattern/instance/PortInstance.cpp include/traact/pattern/instance/PortInstance.h include/traact/pattern/instance/GraphInstance.cpp include/traact/pattern/instance/GraphInstance.h include/traact/util/Utils.h include/traact/util/Utils.cpp include/traact/serialization/JsonPattern.h include/traact/serialization/JsonPatternInstance.h include/traact/serialization/JsonPort.h include/traact/serialization/JsonPortInstance.h include/traact/serialization/JsonSpatialPattern.h include/traact/serialization/JsonGraphInstance.h)

# rest of source code
#set(LIB_SOURCES )

# conan is required hard, library name, version and description are configured from conan
if (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
else ()
    message(FATAL_ERROR "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()

project(${CONAN_PACKAGE_NAME} VERSION ${CONAN_PACKAGE_VERSION})

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

add_library(${CONAN_PACKAGE_NAME} SHARED
        ${LIB_HEADERS}
        #${LIB_SOURCES}
        )

target_link_libraries(${CONAN_PACKAGE_NAME} PUBLIC
        CONAN_PKG::opencv
        CONAN_PKG::TBB
        CONAN_PKG::spdlog
        CONAN_PKG::rttr
        CONAN_PKG::eigen
        CONAN_PKG::Boost
        )

set_target_properties(${CONAN_PACKAGE_NAME} PROPERTIES VERSION ${CONAN_PACKAGE_VERSION})
#set_target_properties(${CONAN_PACKAGE_NAME} PROPERTIES PUBLIC_HEADER ${LIB_HEADERS})

target_include_directories(${CONAN_PACKAGE_NAME} PUBLIC include ${CONAN_INCLUDE_DIRS_NLOHMANN_JSON})
target_include_directories(${CONAN_PACKAGE_NAME} PRIVATE src)

IF (NOT WIN32)
    MESSAGE(STATUS "Set boost::ublas alignment to 16")
    set_target_properties(${CONAN_PACKAGE_NAME} PROPERTIES COMPILE_DEFINITIONS "BOOST_UBLAS_BOUNDED_ARRAY_ALIGN=__attribute__ ((aligned (16)))")
ENDIF (NOT WIN32)

install(TARGETS ${CONAN_PACKAGE_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

add_subdirectory(apps/traactConsole)
add_subdirectory(module_spatial)

if (WITH_TESTS)
    add_subdirectory(test)
endif ()

