/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_FACADE_RawApplicationSyncSink_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_FACADE_RawApplicationSyncSink_H_

#include "traact/buffer/ComponentBuffer.h"
#include "traact/component/Component.h"
#include "traact/traact_plugins.h"
#include <spdlog/spdlog.h>

namespace traact::component::facade {

class RawApplicationSyncSink : public Component {
 public:
    using ConfigCallback = std::function<void(const pattern::instance::PatternInstance &)> ;
    using ValueCallback = std::function<void(buffer::ComponentBuffer &)> ;


    explicit RawApplicationSyncSink(const std::string &name);

    static pattern::Pattern::Ptr GetPattern();

    virtual bool configure(const pattern::instance::PatternInstance &pattern_instance,
                           buffer::ComponentBufferConfig *data) override;

    bool processTimePoint(traact::buffer::ComponentBuffer &data) override;

    bool processTimePointWithInvalid(buffer::ComponentBuffer &data) override;

    bool start() override;
    bool stop() override;

    void setCallback(const ValueCallback &callback);

    void setInvalidCallback(const ValueCallback &callback);

    void setConfigCallback(const ConfigCallback& callback);

 protected:
    ConfigCallback config_callback_;
    ValueCallback callback_;
    ValueCallback invalid_callback_;




};
}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_FACADE_RawApplicationSyncSink_H_
