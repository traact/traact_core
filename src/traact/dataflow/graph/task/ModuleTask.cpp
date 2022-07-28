/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "ModuleTask.h"

namespace traact::dataflow {

ModuleTask::ModuleTask(std::string name, const component::Module::Ptr &module) : module_(module), name_(name) {}
const std::string &ModuleTask::getId() {
    return name_;
}
const component::Module::Ptr &ModuleTask::getModule() const {
    return module_;
}

} // traact