/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_GATE_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_GATE_H_

#include "traact/component/Component.h"
#include "traact/traact_plugins.h"
#include <spdlog/spdlog.h>
#include <traact/pattern/ParameterUtils.h>
#include "traact/buffer/ComponentBuffer.h"
#include "traact/userEvent/UserEvent.h"

namespace traact::component {

template<class T>
class Gate : public Component {
 public:
    using InData = buffer::PortConfig<T, 0>;
    using InEvent = buffer::PortConfig<userEvent::UserEventHeader, 1>;

    using OutData = buffer::PortConfig<T, 0>;

    explicit Gate(const std::string &name) : Component(name) {

    }
    [[nodiscard]] static traact::pattern::Pattern::Ptr GetPattern() {

        std::string pattern_name = fmt::format("Gate_{0}", T::NativeTypeName);

        traact::pattern::Pattern::Ptr
            pattern =
            std::make_shared<traact::pattern::Pattern>(pattern_name, Concurrency::SERIAL,ComponentType::SYNC_SOURCE);

        pattern->addTimeDomain(ComponentType::SYNC_SINK)
            .template addConsumerPort<InData>("input")
            .template addConsumerPort<InEvent>("input_event")
            .template addProducerPort<OutData>("output");


        pattern->addCoordinateSystem("A")
            .addCoordinateSystem("B")
            .addEdge("A", "B", "input")
            .addEdge("A", "B", "output");

        return pattern;
    }

    bool processTimePoint(buffer::ComponentBuffer &data) override {
        data.template getOutput<OutData>() = data.template getInput<InData>();
        return true;
    }

};

}


#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_GATE_H_
