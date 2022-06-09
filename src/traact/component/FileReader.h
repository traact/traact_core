/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_FILEREADER_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_FILEREADER_H_

#include "traact/component/Component.h"
#include "traact/traact_plugins.h"
#include <spdlog/spdlog.h>
#include <traact/pattern/ParameterUtils.h>
#include <traact/util/FileUtil.h>
#include <traact/buffer/ComponentBuffer.h>

namespace traact::component {

template<class T>
class FileReader : public Component {
 public:
    explicit FileReader(const std::string &name, std::string serializer_name) : Component(name,
                                                                                          traact::component::ComponentType::SYNC_SOURCE),
                                                                                serializer_name_(std::move(
                                                                                    serializer_name)) {
    }

    [[nodiscard]] traact::pattern::Pattern::Ptr GetPattern() const override {

        std::string pattern_name = fmt::format("FileReader_{0}_{1}", serializer_name_, T::MetaType);

        traact::pattern::Pattern::Ptr
            pattern =
            std::make_shared<traact::pattern::Pattern>(pattern_name, Concurrency::SERIAL);

        pattern->addProducerPort("output", T::MetaType);
        pattern->addStringParameter("file", "file.json");

        pattern->addCoordinateSystem("A").addCoordinateSystem("B").addEdge("A", "B", "output");

        return pattern;
    }

    bool configure(const nlohmann::json &parameter, buffer::ComponentBufferConfig *data) override {
        bool result = pattern::setValueFromParameter(parameter, "file", filename_, "");
        if (result) {
            if (util::fileExists(filename_, getName())) {
                readValue(data_);
            }
            return true;
        } else {
            SPDLOG_ERROR("{0}: missing file property", getName());
        }

        return false;
    }

    virtual bool readValue(typename T::NativeType &data) = 0;

    bool processTimePoint(buffer::ComponentBuffer &data) override {
        auto &output = data.template getOutput<T>(0);
        output = data_;
        return true;
    }

 protected:
    std::string filename_;
    const std::string serializer_name_;
    typename T::NativeType data_;

 TRAACT_PLUGIN_ENABLE(Component)

};

}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_FILEREADER_H_
