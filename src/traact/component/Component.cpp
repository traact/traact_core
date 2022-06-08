/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "Component.h"
#include <spdlog/spdlog.h>

namespace traact::component {

Component::Component(std::string name, const ComponentType traact_component_type)
    : name_(std::move(name)), traact_component_type_(traact_component_type) {

}

const std::string &Component::getName() const {
    return name_;
}

const ComponentType &Component::getComponentType() const {
    return traact_component_type_;
}

bool Component::configure(const nlohmann::json &parameter, buffer::ComponentBufferConfig *data) {
    return true;
}

bool Component::start() {
    return true;
}

bool Component::stop() {
    return true;
}

bool Component::teardown() {
    return true;
}

bool Component::processTimePoint(buffer::ComponentBuffer &data) {
    return true;
}

void Component::setRequestCallback(const RequestCallback &request_callback) {
    request_callback_ = request_callback;
}

void Component::invalidTimePoint(Timestamp timestamp, size_t mea_idx) {

}

void Component::releaseAsyncCall(Timestamp timestamp, bool valid) {
    if (release_async_callback_) {
        release_async_callback_(timestamp, valid);
    } else {
        SPDLOG_ERROR("release_async_callback_ not set");
    }
}

void Component::setReleaseAsyncCallback(const Component::ReleaseAsyncCallback &release_async_callback) {
    release_async_callback_ = release_async_callback;
}

void Component::setSourceFinishedCallback(const Component::SourceFinishedCallback &finished_callback) {
    finished_callback_ = finished_callback;
}

void Component::setSourceFinished() {
    if (finished_callback_) {
        finished_callback_();
    } else {
        SPDLOG_ERROR("finished_callback_ not set");
    }
}

}
