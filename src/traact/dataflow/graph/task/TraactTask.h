/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_TRAACTTASK_H_
#define TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_TRAACTTASK_H_

#include <memory>
#include <set>
#include "traact/component/ComponentGraph.h"
#include "TraactTaskData.h"

namespace traact::dataflow {


class TraactTask : public std::enable_shared_from_this<TraactTask> {
 public:
    using SharedPtr = std::shared_ptr<TraactTask>;
    using TaskFunction = std::function<void(void)>;

    virtual ~TraactTask() = default;

    void succeed(SharedPtr predecessor);
    void precede(SharedPtr successor);

    void succeedInterTimeStep(SharedPtr predecessor);
    void precedeInterTimeStep(SharedPtr successor);


    const std::set<SharedPtr> &predecessors() const;
    const std::set<SharedPtr> &successors() const;
    const std::set<SharedPtr> &predecessorsInterTimeStep() const;
    const std::set<SharedPtr> &successorsInterTimeStep() const;

    bool isStartPoint() const;
    bool isEndPoint() const;
    virtual bool isSource() const;

    void clear();

    virtual const std::string& getId() = 0;

 private:
    std::set<SharedPtr> predecessors_;
    std::set<SharedPtr> successors_;
    std::set<SharedPtr> predecessors_inter_time_step_;
    std::set<SharedPtr> successors_inter_time_step_;

};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_DATAFLOW_GRAPH_TASK_TRAACTTASK_H_
