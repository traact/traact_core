/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "SourceTask.h"

namespace traact::dataflow {

SourceTask::SourceTask(const component::ComponentGraph::PatternPtr &pattern,
                       const component::ComponentGraph::ComponentPtr &component)
    : pattern_(pattern), component_(component) {}
const std::string &SourceTask::getId() {
    return pattern_->instance_id;
}
const component::ComponentGraph::PatternPtr &SourceTask::getPattern() const {
    return pattern_;
}
const component::ComponentGraph::ComponentPtr &SourceTask::getComponent() const {
    return component_;
}
bool SourceTask::isSource() const {
    return true;
}
} // traact