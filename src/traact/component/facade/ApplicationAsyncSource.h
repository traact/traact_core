/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_FACADE_APPLICATIONASYNCSOURCE_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_FACADE_APPLICATIONASYNCSOURCE_H_

#include <traact/traact.h>
#include <traact/buffer/SourceComponentBuffer.h>
namespace traact::component::facade {

template<typename HeaderType>
class ApplicationAsyncSource : public Component {
 public:
    typedef typename std::shared_ptr<ApplicationAsyncSource<HeaderType> > Ptr;
    typedef typename HeaderType::NativeType NativeType;

    explicit ApplicationAsyncSource(const std::string &name) : Component(name,
                                                                         traact::component::ComponentType::ASYNC_SOURCE) {

    }

    pattern::Pattern::Ptr GetPattern() const {
        std::string pattern_name = "ApplicationAsyncSource_" + std::string(HeaderType::NativeTypeName);
        pattern::Pattern::Ptr
            pattern = std::make_shared<pattern::Pattern>(pattern_name, Concurrency::SERIAL);
        pattern->addProducerPort("output", HeaderType::MetaType);
        pattern->addCoordinateSystem("A").addCoordinateSystem("B").addEdge("A", "B", "output");
        return pattern;
    }

    bool newValue(Timestamp ts, const NativeType &value) {

        try {
            //spdlog::info("acquire buffer");
            auto buffer = request_callback_(ts);
            buffer.wait();
            auto buffer_p = buffer.get();
            if (buffer_p == nullptr)
                return false;

            //spdlog::info("get output");
            auto &new_data = buffer_p->template getOutput<HeaderType>(0);
            //spdlog::info("write value");
            new_data = value;
            //spdlog::info("commit data");
            buffer_p->commit(true);

            return true;
        } catch (std::future_error e) {
            SPDLOG_ERROR(e.what());
            throw e;
        }
    }

    bool start() override {
        spdlog::info("ApplicationAsyncSource got start signal");
        return true;
    }
    bool stop() override {
        spdlog::info("ApplicationAsyncSource got stop signal");
        return true;
    }

 RTTR_ENABLE(Component)

};
}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_FACADE_APPLICATIONASYNCSOURCE_H_
