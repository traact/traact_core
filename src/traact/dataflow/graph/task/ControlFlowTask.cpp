/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "ControlFlowTask.h"

namespace traact {
dataflow::ControlFlowTask::ControlFlowTask(const std::string &name) : name_(name) {}
const std::string &dataflow::ControlFlowTask::getId() {
    return name_;
}

} // traact