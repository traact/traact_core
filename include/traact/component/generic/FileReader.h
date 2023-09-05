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
    explicit FileReader(const std::string &name, std::string serializer_name) : Component(name),
                                                                                serializer_name_(std::move(
                                                                                    serializer_name)) {
    }

    [[nodiscard]] static traact::pattern::Pattern::Ptr GetBasePattern(const std::string& serializer_name) {

        std::string pattern_name = fmt::format("FileReader_{0}_{1}", serializer_name, T::NativeTypeName);

        traact::pattern::Pattern::Ptr
            pattern =
            std::make_shared<traact::pattern::Pattern>(pattern_name, Concurrency::SERIAL, ComponentType::SYNC_SOURCE);

        pattern->addProducerPort("output", T::NativeTypeName);
        pattern->addStringParameter("file", "file.json");

        pattern->addCoordinateSystem("A").addCoordinateSystem("B").addEdge("A", "B", "output");

        return pattern;
    }

    bool configure(const pattern::instance::PatternInstance &pattern_instance, buffer::ComponentBufferConfig *data) override {
        try{
            bool result = pattern::setValueFromParameter(pattern_instance, "file", filename_, "");
            if (result) {
                if (util::fileExists(filename_, getName())) {
                    readValue(data_);
                }
                return true;
            } else {
                SPDLOG_ERROR("{0}: missing file property", getName());
            }
        } catch(std::exception &e){
            SPDLOG_ERROR(e.what());
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
};

}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_FILEREADER_H_
