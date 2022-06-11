cmake_minimum_required(VERSION 3.16)

if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
else ()
    message(FATAL_ERROR "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()

list(APPEND CMAKE_MODULE_PATH ${CONAN_TRAACT_RUN_ENV_ROOT}/cmake)

project(${CONAN_PACKAGE_NAME}_PROJECT VERSION ${CONAN_PACKAGE_VERSION})

include(traact_default_library_setup)

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
        src/traact/component/generic/ApplicationAsyncSource.h
        src/traact/component/generic/ApplicationSyncSink.h
        src/traact/component/PlayerBaseComponent.cpp
        src/traact/component/PlayerBaseComponent.h

        src/traact/component/generic/FileWriter.h
        src/traact/component/generic/FileReader.h
        src/traact/component/generic/FilePlayer.h
        src/traact/component/generic/FileRecorder.h
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
        src/traact/buffer/DataFactory.h src/traact/buffer/BufferUtils.h
        src/traact/buffer/DataBuffer.cpp src/traact/buffer/DataBuffer.h
        src/traact/buffer/DataBufferFactory.h
        src/traact/dataflow/taskflow/TaskFlowUtils.h
        src/traact/util/EnumToString.cpp
        src/traact/util/EnumToString.h
        src/traact/traact_plugins.h
        src/traact/facade/rttr/traact_plugins_rttr.h src/traact/component/generic_components.h src/traact/component/generic/Buffer.h src/traact/component/ComponentFactory.h)


add_library(${CONAN_PACKAGE_NAME} SHARED ${CORE_SRC})

conan_target_link_libraries(${CONAN_PACKAGE_NAME})

install(DIRECTORY "./src/traact" DESTINATION "${CMAKE_INSTALL_PREFIX}/include" FILES_MATCHING
        PATTERN "*.h"
        PATTERN "_internal" EXCLUDE
        )

include(traact_default_library_final)

add_subdirectory(apps)

if (WITH_TESTS)
    add_subdirectory(tests)
endif ()

