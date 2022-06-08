/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/
#include "traact/util/Logging.h"
#include "ModuleComponent.h"

#include <utility>

namespace traact::component {

bool Module::init(ComponentPtr module_component) {
    return true;
}

bool Module::start(ComponentPtr module_component) {
    return true;
}

bool Module::stop(ComponentPtr module_component) {
    return true;
}

bool Module::teardown(ComponentPtr module_component) {
    return true;
}

ModuleComponent::ModuleComponent(std::string name, ComponentType traact_component_type, ModuleType module_type)
    : Component(std::move(name), traact_component_type), module_type_(module_type) {

}

ModuleType ModuleComponent::getModuleType() const {
    return module_type_;
}

void ModuleComponent::setModule(Module::Ptr module) {
    SPDLOG_DEBUG("setting module for module component");
    module_ = std::move(module);
}

bool ModuleComponent::configure(const nlohmann::json &parameter, buffer::ComponentBufferConfig *data) {
    return module_->init(this);
}
bool ModuleComponent::start() {
    return module_->start(this);
}
bool ModuleComponent::stop() {
    return module_->stop(this);
}
bool ModuleComponent::teardown() {
    return module_->teardown(this);
}

}