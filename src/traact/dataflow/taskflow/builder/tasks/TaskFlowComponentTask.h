/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKS_TASKFLOWCOMPONENTTASK_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKS_TASKFLOWCOMPONENTTASK_H_

#include "traact/dataflow/graph/task/ComponentTask.h"

namespace traact::dataflow::taskflow {

class TaskFlowComponentTask {
 public:
    TaskFlowComponentTask(std::shared_ptr<traact::dataflow::ComponentTask> task,
                          std::shared_ptr<TraactTaskData> data);
    void process();
 private:
    component::ComponentGraph::PatternPtr pattern_;
    component::ComponentGraph::ComponentPtr component_;
    std::shared_ptr<TraactTaskData> data_;

};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_BUILDER_TASKS_TASKFLOWCOMPONENTTASK_H_
