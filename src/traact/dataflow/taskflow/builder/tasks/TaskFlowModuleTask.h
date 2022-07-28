/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKS_TASKFLOWMODULETASK_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKS_TASKFLOWMODULETASK_H_

#include "traact/dataflow/graph/task/ModuleTask.h"

namespace traact::dataflow::taskflow {

class TaskFlowModuleTask {
 public:
    TaskFlowModuleTask(std::shared_ptr<traact::dataflow::ModuleTask> task,
    std::shared_ptr<TraactTaskData> data);
    void process();
 private:
    component::Module::Ptr module_;
    std::shared_ptr<TraactTaskData> data_;
};

} // taskflow

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKS_TASKFLOWMODULETASK_H_
