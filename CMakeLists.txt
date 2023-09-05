cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME traact_core)

project(${TARGET_NAME} VERSION "0.0.0" LANGUAGES CXX)

find_package(traact_base)
list(APPEND CMAKE_MODULE_PATH ${traact_base_INCLUDE_DIR})

include(traact_default_library_setup)

set(CORE_PUBLIC_HEADER 
        include/traact/traact.h
        include/traact/component/Component.h
        include/traact/component/ComponentTypes.h
        include/traact/datatypes.h
        include/traact/dataflow/Network.h
        include/traact/pattern/Pattern.h
        include/traact/pattern/Port.h
        include/traact/pattern/CoordinateSystem.h
        include/traact/util/FileUtil.h
        include/traact/pattern/MetaPattern.h
        include/traact/component/ComponentGraph.h
        include/traact/buffer/ComponentBuffer.h
        include/traact/buffer/TimeDomainBuffer.h
        include/traact/pattern/instance/PatternInstance.h
        include/traact/pattern/instance/PortInstance.h
        include/traact/pattern/instance/GraphInstance.h
        include/traact/util/Utils.h
        include/traact/facade/Facade.h
        include/traact/component/generic/ApplicationAsyncSource.h
        include/traact/component/generic/ApplicationSyncSink.h
        include/traact/component/PlayerBaseComponent.h
        include/traact/component/generic/FileWriter.h
        include/traact/component/generic/FileReader.h
        include/traact/component/generic/FilePlayer.h
        include/traact/component/generic/FileRecorder.h
        include/traact/facade/PluginFactory.h
        include/traact/serialization/JsonPattern.h
        include/traact/serialization/JsonPatternInstance.h
        include/traact/serialization/JsonPort.h
        include/traact/serialization/JsonPortInstance.h
        include/traact/util/Logging.h        
        include/traact/serialization/JsonGraphInstance.h
        include/traact/pattern/ParameterUtils.h
        include/traact/component/ModuleComponent.h
        include/traact/buffer/TimeStepBuffer.h
        include/traact/util/Semaphore.h
        include/traact/facade/DefaultFacade.h
        include/traact/buffer/SourceComponentBuffer.h
        include/traact/buffer/DataFactory.h
        include/traact/buffer/BufferUtils.h
        include/traact/buffer/DataBuffer.h
        include/traact/buffer/DataBufferFactory.h
        include/traact/util/EnumToString.h
        include/traact/traact_plugins.h
        include/traact/component/generic_components.h
        include/traact/component/generic/Buffer.h
        include/traact/component/ComponentFactory.h    
        include/traact/util/FpsCounter.h
        include/traact/util/CircularBuffer.h
        include/traact/component/generic/RawApplicationSyncSink.h 
        include/traact/util/ValueWrapper.h 
        include/traact/component/CudaComponent.h         
        include/traact/userEvent/UserEvent.h 
        include/traact/component/generic/Gate.h 
        include/traact/component/generic/FileReaderWriter.h 
        include/traact/userEvent/component/SyncUserEventComponent.h 
        include/traact/pattern/PatternTags.h 
        include/traact/dataflow/state/TimeDomainState.h 
        include/traact/dataflow/state/TimeDomainStateResult.h 
        include/traact/dataflow/state/DataflowState.h
        include/traact/facade/rttr/traact_plugins_rttr.h
        include/traact/dataflow/graph/task/TraactTaskId.h
    )

    

set(CORE_SRC         
        src/traact/util/FileUtil.cpp        
        src/traact/buffer/TimeDomainBuffer.cpp
        src/traact/facade/Facade.cpp
        src/traact/component/PlayerBaseComponent.cpp
        src/traact/util/Logging.cpp
        src/traact/facade/rttr/RTTRPluginFactory.cpp
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
        src/traact/facade/DefaultFacade.cpp
        src/traact/serialization/JsonGraphInstance.cpp
        src/traact/serialization/JsonPortInstance.cpp
        src/traact/serialization/JsonPattern.cpp
        src/traact/serialization/JsonPatternInstance.cpp
        src/traact/serialization/JsonPort.cpp
        src/traact/serialization/JsonTimeDomainManagerConfig.cpp
        src/traact/dataflow/taskflow/TaskFlowNetwork.cpp
        src/traact/buffer/ComponentBuffer.cpp
        src/traact/buffer/SourceComponentBuffer.cpp
        src/traact/dataflow/taskflow/TaskFlowTimeDomain.cpp
        src/traact/buffer/DataBuffer.cpp
        src/traact/util/EnumToString.cpp
        src/traact/dataflow/taskflow/TaskFlowInFlowScheduler.cpp
        src/traact/dataflow/taskflow/TimeDomainClock.cpp
        src/traact/util/FpsCounter.cpp
        src/traact/util/CircularBuffer.cpp
        src/traact/component/generic/RawApplicationSyncSink.cpp 
        src/traact/component/CudaComponent.cpp 
        src/traact/dataflow/graph/cuda/TraactGpuGraph.cu 
        src/traact/dataflow/graph/cuda/TraactGpuGraph.cuh 
        src/traact/dataflow/graph/builder/GraphBuilder.cpp 
        src/traact/dataflow/graph/builder/ComponentGraphBuilder.cpp 
        src/traact/dataflow/graph/cuda/CudaGraphBuilder.cpp 
        src/traact/dataflow/graph/builder/ModuleGraphBuilder.cpp 
        src/traact/dataflow/graph/task/TraactTask.cpp 
        src/traact/dataflow/graph/task/SourceTask.cpp 
        src/traact/dataflow/graph/task/ComponentTask.cpp 
        src/traact/dataflow/taskflow/builder/TraactTaskFlowGraph.cpp 
        src/traact/dataflow/state/DataflowState.cpp 
        src/traact/dataflow/taskflow/builder/TaskFlowTimeStep.cpp 
        src/traact/dataflow/taskflow/builder/TaskFlowConnections.cpp 
        src/traact/dataflow/graph/task/TraactTaskData.cpp 
        src/traact/dataflow/graph/task/ModuleTask.cpp 
        src/traact/dataflow/graph/task/ControlFlowTask.cpp 
        src/traact/dataflow/graph/cuda/TraactCudaTask.cpp 
        src/traact/dataflow/graph/TraactGraphBuilder.cpp 
        src/traact/dataflow/graph/TraactGraph.cpp 
        src/traact/dataflow/taskflow/builder/TaskFlowGraphBuilder.cpp 
        src/traact/dataflow/taskflow/TraactTaskFlowBuilder.cpp 
        src/traact/dataflow/taskflow/builder/TaskFlowStandardTasks.cpp 
        src/traact/dataflow/taskflow/builder/tasks/TaskFlowComponentTask.cpp 
        src/traact/dataflow/taskflow/builder/tasks/TaskFlowSourceTask.cpp 
        src/traact/dataflow/taskflow/builder/tasks/TaskFlowModuleTask.cpp 
        src/traact/dataflow/graph/cuda/TaskFlowCudaGraphBuilder.cpp 
        src/traact/traact.cpp src/traact/userEvent/UserEvent.cpp 
        src/traact/component/generic/FileReaderWriter.cpp 
        src/traact/userEvent/component/SyncUserEventComponent.cpp 
        src/traact/dataflow/state/TimeDomainState.cpp 
        src/traact/dataflow/state/TimeDomainStateResult.cpp 
        src/traact/dataflow/taskflow/TaskFlowProfiler.cpp
        src/traact/dataflow/taskflow/TaskFlowUtils.h
        src/traact/dataflow/taskflow/TaskFlowNetwork.h                
        src/traact/dataflow/taskflow/TaskFlowTaskFunctions.h
        src/traact/dataflow/taskflow/TaskFlowTimeDomain.h
        src/traact/dataflow/taskflow/TaskFlowInFlowScheduler.h
        src/traact/dataflow/taskflow/TimeDomainClock.h
        src/traact/dataflow/graph/builder/GraphBuilder.h 
        src/traact/dataflow/graph/builder/ComponentGraphBuilder.h 
        src/traact/dataflow/graph/cuda/CudaGraphBuilder.h 
        src/traact/dataflow/graph/builder/ModuleGraphBuilder.h 
        src/traact/dataflow/graph/task/TraactTask.h 
        src/traact/dataflow/graph/task/SourceTask.h 
        src/traact/dataflow/graph/task/ComponentTask.h 
        src/traact/dataflow/taskflow/builder/TraactTaskFlowGraph.h         
        src/traact/dataflow/taskflow/builder/TaskFlowTimeStep.h 
        src/traact/dataflow/taskflow/builder/TaskFlowConnections.h 
        src/traact/dataflow/graph/task/TraactTaskData.h 
        src/traact/dataflow/graph/task/ModuleTask.h 
        src/traact/dataflow/graph/task/ControlFlowTask.h 
        src/traact/dataflow/graph/cuda/TraactCudaTask.h 
        src/traact/dataflow/graph/TraactGraphBuilder.h 
        src/traact/dataflow/graph/TraactGraph.h 
        src/traact/dataflow/taskflow/builder/TaskFlowGraphBuilder.h 
        src/traact/dataflow/taskflow/TraactTaskFlowBuilder.h 
        src/traact/dataflow/taskflow/builder/TaskFlowStandardTasks.h 
        src/traact/dataflow/taskflow/builder/tasks/TaskFlowComponentTask.h 
        src/traact/dataflow/taskflow/builder/tasks/TaskFlowSourceTask.h 
        src/traact/dataflow/taskflow/builder/tasks/TaskFlowModuleTask.h 
        src/traact/dataflow/graph/cuda/TaskFlowCudaGraphBuilder.h 
        src/traact/dataflow/taskflow/TaskFlowProfiler.h                 
        src/traact/facade/rttr/RTTRPluginFactory.h
        )


add_library(${TARGET_NAME} SHARED ${CORE_SRC})




find_package(cppfs)
find_package(fmt)
find_package(nlohmann_json)
find_package(re2)
find_package(rttr)
find_package(spdlog)
find_package(Taskflow)
find_package(tclap)

target_link_libraries(${TARGET_NAME} cppfs::cppfs fmt::fmt nlohmann_json::nlohmann_json re2::re2 rttr::rttr spdlog::spdlog Taskflow::Taskflow tclap::tclap)

generate_export_header(${TARGET_NAME} EXPORT_FILE_NAME ${CMAKE_CURRENT_SOURCE_DIR}/include/traact/traact_core_export.h)

target_include_directories(${TARGET_NAME} PUBLIC
$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

include(traact_default_library_final)



add_subdirectory(apps)

if (WITH_TESTS)
    add_subdirectory(tests)
endif ()

