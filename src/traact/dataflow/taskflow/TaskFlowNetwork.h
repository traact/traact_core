/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWNETWORK_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWNETWORK_H_

#include "traact/dataflow/Network.h"
#include <atomic>
#include "TaskFlowTimeDomain.h"

namespace traact::dataflow {
class TRAACT_CORE_EXPORT TaskFlowNetwork : public Network {
 public:
    TaskFlowNetwork() = default;
    ~TaskFlowNetwork() override;

    bool start() override;
    bool stop() override;

    void parameterChanged(const std::string &instance_id) override;
    virtual DataflowState::SharedPtr getDataflowState() override;
 private:
    tf::Executor executor_;
    DataflowState::SharedPtr dataflow_state_;
    std::vector<std::unique_ptr<TaskFlowTimeDomain>> task_flow_time_domains_;

    void init();

};
}

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_TASKFLOW_TASKFLOWNETWORK_H_
