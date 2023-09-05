/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_BUFFER_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_BUFFER_H_

#include "traact/component/Component.h"
#include "traact/traact_plugins.h"
#include <spdlog/spdlog.h>
#include <traact/pattern/ParameterUtils.h>
#include "traact/buffer/ComponentBuffer.h"

namespace traact::component {

template<class T>
class Buffer : public Component {
 public:
    using DataIn = buffer::PortConfig<T, 0>;
    using DataOut = buffer::PortConfig<T, 0>;

    explicit Buffer(const std::string &name) : Component(name) {

    }
    [[nodiscard]] static traact::pattern::Pattern::Ptr GetPattern() {

        std::string pattern_name = fmt::format("Buffer_{0}", T::NativeTypeName);

        traact::pattern::Pattern::Ptr
            pattern =
            std::make_shared<traact::pattern::Pattern>(pattern_name, Concurrency::SERIAL,ComponentType::SYNC_SOURCE);

        pattern->addTimeDomain(ComponentType::SYNC_SINK)
        .template addConsumerPort<DataIn>("input", 0)
        .template addProducerPort<DataOut>("output", 1);


        pattern->addCoordinateSystem("A")
            .addCoordinateSystem("B")
            .addEdge("A", "B", "input")
            .addEdge("A", "B", "output");

        return pattern;
    }

    virtual bool processTimePoint(buffer::ComponentBuffer &data) override {
        if(data.getTimeDomain() == 0){

            value_ = {data.template getInput<DataIn>()};
            header_ = {data.template getInputHeader<DataIn>()};
        } else {
            if(value_.has_value()) {
                data.template getOutput<DataOut>() = value_.value();
                data.template getOutputHeader<DataOut>() = header_.value();
            }
        }

        return Component::processTimePoint(data);
    }
    virtual bool processTimePointWithInvalid(buffer::ComponentBuffer &data) override {
        value_ = {};
        header_ = {};
        return true;
    }

 protected:
    std::optional<typename T::NativeType> value_;
    std::optional<T> header_;
};

}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_BUFFER_H_
