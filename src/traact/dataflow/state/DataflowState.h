/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_STATE_DATAFLOWSTATE_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_STATE_DATAFLOWSTATE_H_

#include <memory>
#include <map>
#include "TaskState.h"
namespace traact::dataflow {

class DataflowState {
 public:
    using SharedPtr = std::shared_ptr<DataflowState>;
    using ConstSharedPtr = std::shared_ptr<const DataflowState>;

    TaskState& getTaskState(const std::string& task_id);
 private:
    std::map<std::string, TaskState> task_states_;


};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_STATE_DATAFLOWSTATE_H_
