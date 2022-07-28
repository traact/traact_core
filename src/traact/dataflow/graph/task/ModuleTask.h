/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_MODULETASK_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_MODULETASK_H_

#include "TraactTask.h"
#include "traact/component/ModuleComponent.h"
namespace traact::dataflow {

class ModuleTask : public TraactTask {
 public:
    explicit ModuleTask(std::string name, const component::Module::Ptr &module);
    virtual const std::string &getId() override;
    const component::Module::Ptr &getModule() const;
 protected:
    std::string name_;
    component::Module::Ptr module_;
};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_MODULETASK_H_
