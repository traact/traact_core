cmake_minimum_required(VERSION 3.9)

option(WITH_TESTS "Build Tests" ON)
option(WITH_COVERAGE "" ON)

set(CMAKE_CXX_STANDARD 17)

# public header
set(SPATIAL_MODULE_HEADER
        src/traact/component/spatial/core/MultiplicationComponent.h
        src/traact/component/spatial/core/MultiplicationComponent.cpp src/traact/spatial/spatialPlugin.h src/traact/spatial/spatialPlugin.cpp src/traact/component/spatial/util/Pose6DTestSource.cpp src/traact/component/spatial/util/Pose6DTestSource.h src/traact/component/spatial/util/Pose6DPrint.cpp src/traact/component/spatial/util/Pose6DPrint.h)

# rest of source code
#set(LIB_SOURCES )


add_library(spatial_module SHARED
        ${SPATIAL_MODULE_HEADER}
        #${LIB_SOURCES}
        )

target_link_libraries(spatial_module PUBLIC
        ${CONAN_PACKAGE_NAME}
        CONAN_PKG::TBB
        CONAN_PKG::spdlog
        CONAN_PKG::rttr
        CONAN_PKG::eigen
        )


target_include_directories(spatial_module PRIVATE include)
#target_include_directories(${CONAN_PACKAGE_NAME} PRIVATE src)

install(TARGETS spatial_module
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})


