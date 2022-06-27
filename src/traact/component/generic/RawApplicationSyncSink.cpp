/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "RawApplicationSyncSink.h"
#include "traact/traact.h"

namespace traact::component::facade {

RawApplicationSyncSink::RawApplicationSyncSink(const std::string &name) : Component(name) {

}
pattern::Pattern::Ptr RawApplicationSyncSink::GetPattern() {
    std::string pattern_name = "RawApplicationSyncSink";
    pattern::Pattern::Ptr
        pattern = std::make_shared<pattern::Pattern>(pattern_name, Concurrency::SERIAL, ComponentType::SYNC_SINK);
    return pattern;
}
bool RawApplicationSyncSink::processTimePoint(buffer::ComponentBuffer &data) {

    if (callback_) {
        callback_(data);
    } else {
        SPDLOG_WARN("RawApplicationSyncSink {0}: missing callback function", getName());
    }

    return true;
}
bool RawApplicationSyncSink::processTimePointWithInvalid(buffer::ComponentBuffer &data) {
    if (invalid_callback_){
        invalid_callback_(data);
    }

    return true;
}
bool RawApplicationSyncSink::start() {
    SPDLOG_INFO("RawApplicationSyncSink got start signal");
    return true;

}
bool RawApplicationSyncSink::stop() {
    SPDLOG_INFO("RawApplicationSyncSink got stop signal");
    return true;
}
void RawApplicationSyncSink::setCallback(const RawApplicationSyncSink::ValueCallback &callback) {
    callback_ = callback;
}
void RawApplicationSyncSink::setInvalidCallback(const RawApplicationSyncSink::ValueCallback &callback) {
    invalid_callback_ = callback;
}
void RawApplicationSyncSink::setConfigCallback(const RawApplicationSyncSink::ConfigCallback &callback) {
    config_callback_ = callback;
}
bool RawApplicationSyncSink::configure(const pattern::instance::PatternInstance &pattern_instance,
                                       buffer::ComponentBufferConfig *data) {
    if(config_callback_){
        config_callback_(pattern_instance);
    }
    return true;
}

CREATE_TRAACT_COMPONENT_FACTORY(RawApplicationSyncSink)


}

BEGIN_TRAACT_PLUGIN_REGISTRATION
    REGISTER_DEFAULT_COMPONENT(traact::component::facade::RawApplicationSyncSink)
END_TRAACT_PLUGIN_REGISTRATION