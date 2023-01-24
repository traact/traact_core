/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_APPLICATIONASYNCSOURCE_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_APPLICATIONASYNCSOURCE_H_

#include "traact/component/Component.h"
#include "traact/traact_plugins.h"
#include <spdlog/spdlog.h>
#include "traact/buffer/SourceComponentBuffer.h"

namespace traact::component::facade {

template<typename HeaderType>
class ApplicationAsyncSource : public Component {
 public:
    using Ptr = typename std::shared_ptr<ApplicationAsyncSource<HeaderType>>;
    using NativeType = typename HeaderType::NativeType;

    explicit ApplicationAsyncSource(const std::string &name) : Component(name) {

    }

    static pattern::Pattern::Ptr GetPattern() {
        static const std::string base_name = "ApplicationAsyncSource";
        std::string pattern_name = base_name + "_" + std::string(HeaderType::NativeTypeName);
        auto display_name = fmt::format("{0} ({1})","Application Source (async)", HeaderType::MetaType);
        pattern::Pattern::Ptr
            pattern = std::make_shared<pattern::Pattern>(pattern_name, Concurrency::SERIAL, ComponentType::ASYNC_SOURCE);
        pattern->addProducerPort("output", HeaderType::NativeTypeName)
            .setDisplayName(display_name)
            .setDescription("Provide data from the user application to the dataflow network")
            .addTag(pattern::tags::kApplication)
            .addTag(pattern::tags::kTemplated)
            .addCoordinateSystem("A")
            .addCoordinateSystem("B")
            .addEdge("A", "B", "output");
        return pattern;
    }

    bool newValue(Timestamp ts, const NativeType &value) {

        try {
            //SPDLOG_INFO("acquire buffer");
            auto buffer = request_callback_(ts);
            buffer.wait();
            auto buffer_p = buffer.get();
            if (buffer_p == nullptr)
                return false;

            //SPDLOG_INFO("get output");
            auto &new_data = buffer_p->template getOutput<HeaderType>(0);
            //SPDLOG_INFO("write value");
            new_data = value;
            //SPDLOG_INFO("commit data");
            buffer_p->commit(true);

            return true;
        } catch (std::future_error& e) {
            SPDLOG_ERROR(e.what());
            throw e;
        }
    }

    bool start() override {
        SPDLOG_INFO("ApplicationAsyncSource got start signal");
        return true;
    }
    bool stop() override {
        SPDLOG_INFO("ApplicationAsyncSource got stop signal");
        return true;
    }
};
}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_APPLICATIONASYNCSOURCE_H_
