/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_COMPONENTTASK_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_COMPONENTTASK_H_

#include "TraactTask.h"

namespace traact::dataflow {

class ComponentTask : public TraactTask{
 public:
    ComponentTask(const component::ComponentGraph::PatternPtr &pattern,
                 const component::ComponentGraph::ComponentPtr &component);

    virtual const std::string &getId() override;
    const component::ComponentGraph::PatternPtr &getPattern() const;
    const component::ComponentGraph::ComponentPtr &getComponent() const;
 protected:
    component::ComponentGraph::PatternPtr pattern_;
    component::ComponentGraph::ComponentPtr component_;
};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_COMPONENTTASK_H_
