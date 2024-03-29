/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/
#include <traact/util/Logging.h>
#include <traact/component/ModuleComponent.h>

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
void Module::processTimePoint() {

}
bool Module::useConstraints() const {
    return false;
}

ModuleComponent::ModuleComponent(std::string name, ModuleType module_type)
    : Component(std::move(name)), module_type_(module_type) {

}

ModuleType ModuleComponent::getModuleType() const {
    return module_type_;
}

void ModuleComponent::setModule(Module::Ptr module) {
    SPDLOG_DEBUG("setting module for module component");
    module_ = std::move(module);
}

bool ModuleComponent::configure(const pattern::instance::PatternInstance &pattern_instance, buffer::ComponentBufferConfig *data) {
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