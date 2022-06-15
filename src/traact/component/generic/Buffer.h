/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_BUFFER_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_BUFFER_H_

#include "traact/component/Component.h"
#include "traact/traact_plugins.h"
#include <spdlog/spdlog.h>
#include <traact/pattern/ParameterUtils.h>

namespace traact::component {

template<class T>
class Buffer : public Component {
 public:
    using DataPort = buffer::PortConfig<T, 0>;

    explicit Buffer(const std::string &name) : Component(name) {

    }
    [[nodiscard]] static traact::pattern::Pattern::Ptr GetPattern() {

        std::string pattern_name = fmt::format("Buffer_{0}", T::NativeTypeName);

        traact::pattern::Pattern::Ptr
            pattern =
            std::make_shared<traact::pattern::Pattern>(pattern_name, Concurrency::SERIAL,ComponentType::SYNC_SOURCE);

        pattern->addTimeDomain(ComponentType::SYNC_SINK)
        .template addProducerPort<DataPort>("output",0)
        .template addConsumerPort<DataPort>("input",1);




        pattern->addCoordinateSystem("A")
            .addCoordinateSystem("B")
            .addEdge("A", "B", "output");

        return pattern;
    }



};

}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_BUFFER_H_
