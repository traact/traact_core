/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_FACADE_APPLICATIONSYNCSINK_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_FACADE_APPLICATIONSYNCSINK_H_

#include "traact/buffer/ComponentBuffer.h"
#include "traact/component/Component.h"
#include "traact/traact_plugins.h"
#include <spdlog/spdlog.h>

namespace traact::component::facade {

template<typename HeaderType>
class ApplicationSyncSink : public Component {
 public:
    typedef typename std::shared_ptr<ApplicationSyncSink<HeaderType> > Ptr;
    typedef typename HeaderType::NativeType NativeType;
    typedef typename std::function<void(Timestamp, const NativeType &)> NewValueCallback;
    typedef typename std::function<void(Timestamp)> InvalidTimestampCallback;

    explicit ApplicationSyncSink(const std::string &name) : Component(name) {

    }

    static pattern::Pattern::Ptr GetPattern() {
        std::string pattern_name = "ApplicationSyncSink_" + std::string(HeaderType::NativeTypeName);
        pattern::Pattern::Ptr
            pattern = std::make_shared<pattern::Pattern>(pattern_name, Concurrency::SERIAL, ComponentType::SYNC_SINK);
        pattern->addConsumerPort("input", HeaderType::MetaType);
        pattern->addCoordinateSystem("A").addCoordinateSystem("B").addEdge("A", "B", "input");
        return pattern;
    }

    bool processTimePoint(traact::buffer::ComponentBuffer &data) override {

        if (callback_) {
            const auto &input = data.template getInput<HeaderType>(0);
            callback_(data.getTimestamp(), input);
        } else {
            SPDLOG_WARN("ApplicationSyncSink {0}: missing callback function", getName());
        }

        return true;
    }

    bool processTimePointWithInvalid(buffer::ComponentBuffer &data) override {
        if (invalid_callback_)
            invalid_callback_(data.getTimestamp());
        return true;
    }

    bool start() override {
        SPDLOG_INFO("ApplicationSyncSink got start signal");
        return true;

    }
    bool stop() override {
        SPDLOG_INFO("ApplicationSyncSink got stop signal");
        return true;
    }

    void setCallback(const NewValueCallback &callback) {
        callback_ = callback;
    }

    void setInvalidCallback(const InvalidTimestampCallback &callback) {
        invalid_callback_ = callback;
    }

 protected:
    NewValueCallback callback_;
    InvalidTimestampCallback invalid_callback_;



};
}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_FACADE_APPLICATIONSYNCSINK_H_
