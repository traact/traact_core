/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "TraactTask.h"

namespace traact::dataflow {

const std::set<TraactTask::SharedPtr> &TraactTask::predecessors() const {
    return predecessors_;
}

const std::set<TraactTask::SharedPtr> &TraactTask::successors() const {
    return successors_;
}

void TraactTask::succeed(TraactTask::SharedPtr predecessor) {
    predecessor->successors_.emplace(shared_from_this());
    predecessors_.emplace(predecessor);
}

void TraactTask::precede(TraactTask::SharedPtr successor) {
    successor->predecessors_.emplace(shared_from_this());
    successors_.emplace(successor);
}

bool TraactTask::isStartPoint() const {
    return predecessors_.empty();
}

bool TraactTask::isEndPoint() const {
    return successors_.empty();
}
void TraactTask::succeedInterTimeStep(TraactTask::SharedPtr predecessor) {
    predecessor->successors_inter_time_step_.emplace(shared_from_this());
    predecessors_inter_time_step_.emplace(predecessor);
}
void TraactTask::precedeInterTimeStep(TraactTask::SharedPtr successor) {
    successor->predecessors_inter_time_step_.emplace(shared_from_this());
    successors_inter_time_step_.emplace(successor);
}
const std::set<TraactTask::SharedPtr> &TraactTask::predecessorsInterTimeStep() const {
    return predecessors_inter_time_step_;
}
const std::set<TraactTask::SharedPtr> &TraactTask::successorsInterTimeStep() const {
    return successors_inter_time_step_;
}
void TraactTask::clear() {
    successors_.clear();
    predecessors_.clear();
    successors_inter_time_step_.clear();
    predecessors_inter_time_step_.clear();

}

} // traact