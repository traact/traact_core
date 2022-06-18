/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWNETWORK_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWNETWORK_H_

#include "traact/dataflow/Network.h"
#include <atomic>
#include "TaskFlowGraph.h"

namespace traact::dataflow {
class TRAACT_CORE_EXPORT TaskFlowNetwork : public Network {
 public:
    TaskFlowNetwork() = default;
    TaskFlowNetwork(const TaskFlowNetwork&) = default;
    TaskFlowNetwork(TaskFlowNetwork&&) = default;
    TaskFlowNetwork& operator=(const TaskFlowNetwork&) = default;
    TaskFlowNetwork& operator=(TaskFlowNetwork&&) = default;

    ~TaskFlowNetwork() override;
    bool start() override;

    bool stop() override;
 private:
    std::vector<std::shared_ptr<TaskFlowGraph>> task_graphs_;
    bool is_running_{false};

};
}

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWNETWORK_H_
