/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_TRAACTTASKID_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_TRAACTTASKID_H_

#include <fmt/format.h>
#include <map>
namespace traact::dataflow::task_util {

enum class TaskType : int {
    CONTROL_FLOW = 0,
    SEAM_ENTRY,
    SEAM_START,
    SEAM_END,
    SOURCE,
    COMPONENT,
    MODULE,
    CUDA_COMPONENT,
    CUDA_FLOW,
    INVALID,
    COUNT

};

static const constexpr char* kComponent = "Component";
static const constexpr char* kControlFlow = "ControlFlow";
static const constexpr char* kModule = "Module";
static const constexpr char* kSource = "Source";
static const constexpr char* kCudaComponent = "CudaComponent";
static const constexpr char* kCudaFlow = "CudaFlow";

static const constexpr char *kSeamEntry{"SeamEntry"};
static const constexpr char *kSeamStart{"SeamStart"};
static const constexpr char *kSeamEnd{"SeamEnd"};

static const constexpr char *kGlobalStart{"GlobalStart"};
static const constexpr char *kGlobalEnd{"GlobalEnd"};

static const constexpr char *kTimeStepStart{"TimeStepStart"};
static const constexpr char *kTimeStepEnd{"TimeStepEnd"};

static const constexpr char *kTimeStepSelf{"TimeStepSelf"};

static constexpr TaskType taskTypeNameToEnum(const char* name){
    if(strcmp(name, kComponent) == 0) {
        return TaskType::COMPONENT;
    } else if(strcmp(name, kControlFlow) == 0) {
        return TaskType::CONTROL_FLOW;
    }else if(strcmp(name, kSeamEntry) == 0) {
        return TaskType::SEAM_ENTRY;
    }else if(strcmp(name, kSeamStart) == 0) {
        return TaskType::SEAM_START;
    }else if(strcmp(name, kSeamEnd) == 0) {
        return TaskType::SEAM_END;
    } else if(strcmp(name, kModule) == 0) {
        return TaskType::MODULE;
    }else if(strcmp(name, kSource) == 0) {
        return TaskType::SOURCE;
    }else if(strcmp(name, kCudaComponent) == 0) {
        return TaskType::CUDA_COMPONENT;
    }else if(strcmp(name, kCudaFlow) == 0) {
        return TaskType::CUDA_FLOW;
    }else  {
        return TaskType::INVALID;
    }
};

static constexpr const char* taskTypeEnumToName(TaskType type){
    switch (type) {
        case TaskType::CONTROL_FLOW: return kControlFlow;
        case TaskType::SEAM_ENTRY: return kSeamEntry;
        case TaskType::SEAM_START: return kSeamStart;
        case TaskType::SEAM_END: return kSeamEnd;
        case TaskType::COMPONENT: return kComponent;
        case TaskType::MODULE: return kModule;
        case TaskType::SOURCE: return kSource;
        case TaskType::CUDA_COMPONENT: return kCudaComponent;
        case TaskType::CUDA_FLOW: return kCudaFlow;
        case TaskType::INVALID:
        default:return "invalid";
    }
};

static inline std::string getTaskId(const char* type, const char* name, int time_step){
    return fmt::format("{0}_{1}_{2}", type, time_step, name);
}

}
#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_TRAACTTASKID_H_
