/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_SOURCETASK_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_SOURCETASK_H_

#include "ComponentTask.h"

namespace traact::dataflow {

class SourceTask : public TraactTask{
 public:
    SourceTask(const component::ComponentGraph::PatternPtr &pattern,
               const component::ComponentGraph::ComponentPtr &component);
    virtual const std::string &getId() override;
    const component::ComponentGraph::PatternPtr &getPattern() const;
    const component::ComponentGraph::ComponentPtr &getComponent() const;
    virtual bool isSource() const override;
 private:
    component::ComponentGraph::PatternPtr pattern_;
    component::ComponentGraph::ComponentPtr component_;

};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_SOURCETASK_H_
