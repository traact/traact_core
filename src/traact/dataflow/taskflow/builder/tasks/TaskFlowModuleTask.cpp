/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TaskFlowModuleTask.h"

namespace traact::dataflow::taskflow {
TaskFlowModuleTask::TaskFlowModuleTask(std::shared_ptr<traact::dataflow::ModuleTask> task,
                                       std::shared_ptr<TraactTaskData> data) : module_(task->getModule()), data_(std::move(data)){

}
void TaskFlowModuleTask::process() {
    module_->processTimePoint();
}
} // taskflow