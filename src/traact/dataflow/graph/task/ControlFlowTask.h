/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_CONTROLFLOWTASK_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_CONTROLFLOWTASK_H_

#include "TraactTask.h"

namespace traact::dataflow {

class ControlFlowTask : public TraactTask{
 public:
    ControlFlowTask(const std::string &name);
    virtual const std::string &getId() override;
 protected:
    std::string name_;

};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_CONTROLFLOWTASK_H_
