/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "ComponentTask.h"

namespace traact::dataflow {
ComponentTask::ComponentTask(const component::ComponentGraph::PatternPtr &pattern,
                           const component::ComponentGraph::ComponentPtr &component) : pattern_(pattern), component_(component) {}

const std::string &ComponentTask::getId() {
    return pattern_->instance_id;
}
const component::ComponentGraph::PatternPtr &ComponentTask::getPattern() const {
    return pattern_;
}
const component::ComponentGraph::ComponentPtr &ComponentTask::getComponent() const {
    return component_;
}

} // traact