cmake_minimum_required(VERSION 3.16)

if (EXISTS ${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_CURRENT_BINARY_DIR}/conanbuildinfo.cmake)
else ()
    message(FATAL_ERROR "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
endif ()

list(APPEND CMAKE_MODULE_PATH ${CONAN_TRAACT_RUN_ENV_ROOT}/cmake)

project(${CONAN_PACKAGE_NAME}_PROJECT VERSION ${CONAN_PACKAGE_VERSION} LANGUAGES CXX CUDA)


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
        src/traact/buffer/ComponentBuffer.cpp
        src/traact/buffer/SourceComponentBuffer.cpp
        src/traact/buffer/SourceComponentBuffer.h
        src/traact/dataflow/taskflow/TaskFlowTaskFunctions.h
        src/traact/dataflow/taskflow/TaskFlowTimeDomain.cpp
        src/traact/dataflow/taskflow/TaskFlowTimeDomain.h
        src/traact/buffer/DataFactory.h
        src/traact/buffer/BufferUtils.h
        src/traact/buffer/DataBuffer.cpp
        src/traact/buffer/DataBuffer.h
        src/traact/buffer/DataBufferFactory.h
        src/traact/dataflow/taskflow/TaskFlowUtils.h
        src/traact/util/EnumToString.cpp
        src/traact/util/EnumToString.h
        src/traact/traact_plugins.h
        src/traact/facade/rttr/traact_plugins_rttr.h
        src/traact/component/generic_components.h
        src/traact/component/generic/Buffer.h
        src/traact/component/ComponentFactory.h
        src/traact/dataflow/taskflow/TaskFlowInFlowScheduler.cpp
        src/traact/dataflow/taskflow/TaskFlowInFlowScheduler.h
        src/traact/dataflow/taskflow/TimeDomainClock.cpp
        src/traact/dataflow/taskflow/TimeDomainClock.h
        src/traact/util/FpsCounter.cpp
        src/traact/util/FpsCounter.h
        src/traact/util/CircularBuffer.cpp
        src/traact/util/CircularBuffer.h
        src/traact/component/generic/RawApplicationSyncSink.h src/traact/component/generic/RawApplicationSyncSink.cpp src/traact/util/ValueWrapper.h src/traact/component/CudaComponent.cpp src/traact/component/CudaComponent.h src/traact/dataflow/graph/cuda/TraactGpuGraph.cu src/traact/dataflow/graph/cuda/TraactGpuGraph.cuh src/traact/dataflow/graph/builder/GraphBuilder.cpp src/traact/dataflow/graph/builder/GraphBuilder.h src/traact/dataflow/graph/builder/ComponentGraphBuilder.cpp src/traact/dataflow/graph/builder/ComponentGraphBuilder.h src/traact/dataflow/graph/cuda/CudaGraphBuilder.cpp src/traact/dataflow/graph/cuda/CudaGraphBuilder.h src/traact/dataflow/graph/builder/ModuleGraphBuilder.cpp src/traact/dataflow/graph/builder/ModuleGraphBuilder.h src/traact/dataflow/graph/task/TraactTask.cpp src/traact/dataflow/graph/task/TraactTask.h src/traact/dataflow/graph/task/SourceTask.cpp src/traact/dataflow/graph/task/SourceTask.h src/traact/dataflow/graph/task/ComponentTask.cpp src/traact/dataflow/graph/task/ComponentTask.h src/traact/dataflow/taskflow/builder/TraactTaskFlowGraph.cpp src/traact/dataflow/taskflow/builder/TraactTaskFlowGraph.h src/traact/dataflow/state/DataflowState.cpp src/traact/dataflow/state/DataflowState.h src/traact/dataflow/taskflow/builder/TaskFlowTimeStep.cpp src/traact/dataflow/taskflow/builder/TaskFlowTimeStep.h src/traact/dataflow/taskflow/builder/TaskFlowConnections.cpp src/traact/dataflow/taskflow/builder/TaskFlowConnections.h src/traact/dataflow/graph/task/TraactTaskData.cpp src/traact/dataflow/graph/task/TraactTaskData.h src/traact/dataflow/graph/task/ModuleTask.cpp src/traact/dataflow/graph/task/ModuleTask.h src/traact/dataflow/graph/task/ControlFlowTask.cpp src/traact/dataflow/graph/task/ControlFlowTask.h src/traact/dataflow/graph/cuda/TraactCudaTask.cpp src/traact/dataflow/graph/cuda/TraactCudaTask.h src/traact/dataflow/graph/TraactGraphBuilder.cpp src/traact/dataflow/graph/TraactGraphBuilder.h src/traact/dataflow/graph/TraactGraph.cpp src/traact/dataflow/graph/TraactGraph.h src/traact/dataflow/taskflow/builder/TaskFlowGraphBuilder.cpp src/traact/dataflow/taskflow/builder/TaskFlowGraphBuilder.h src/traact/dataflow/taskflow/TraactTaskFlowBuilder.cpp src/traact/dataflow/taskflow/TraactTaskFlowBuilder.h src/traact/dataflow/taskflow/builder/TaskFlowStandardTasks.cpp src/traact/dataflow/taskflow/builder/TaskFlowStandardTasks.h src/traact/dataflow/taskflow/builder/tasks/TaskFlowComponentTask.cpp src/traact/dataflow/taskflow/builder/tasks/TaskFlowComponentTask.h src/traact/dataflow/taskflow/builder/tasks/TaskFlowSourceTask.cpp src/traact/dataflow/taskflow/builder/tasks/TaskFlowSourceTask.h src/traact/dataflow/taskflow/builder/tasks/TaskFlowModuleTask.cpp src/traact/dataflow/taskflow/builder/tasks/TaskFlowModuleTask.h src/traact/dataflow/graph/cuda/TaskFlowCudaGraphBuilder.cpp src/traact/dataflow/graph/cuda/TaskFlowCudaGraphBuilder.h src/traact/traact.cpp src/traact/userEvent/UserEvent.cpp src/traact/userEvent/UserEvent.h src/traact/component/generic/Gate.h src/traact/component/generic/FileReaderWriter.cpp src/traact/component/generic/FileReaderWriter.h src/traact/userEvent/component/SyncUserEventComponent.cpp src/traact/userEvent/component/SyncUserEventComponent.h src/traact/pattern/PatternTags.h src/traact/dataflow/state/TimeDomainState.cpp src/traact/dataflow/state/TimeDomainState.h src/traact/dataflow/state/TimeDomainStateResult.cpp src/traact/dataflow/state/TimeDomainStateResult.h src/traact/dataflow/taskflow/TaskFlowProfiler.cpp src/traact/dataflow/taskflow/TaskFlowProfiler.h src/traact/dataflow/graph/task/TraactTaskId.h)


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

