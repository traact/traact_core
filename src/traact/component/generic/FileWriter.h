/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_FILEWRITER_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_FILEWRITER_H_

#include "traact/component/Component.h"
#include "traact/buffer/ComponentBuffer.h"
#include "traact/traact_plugins.h"
#include <spdlog/spdlog.h>
#include <traact/pattern/ParameterUtils.h>

namespace traact::component {

template<class T>
class FileWriter : public Component {
 public:
    explicit FileWriter(const std::string &name, std::string serializer_name) : Component(name),
                                                                                       serializer_name_(std::move(serializer_name)) {
    }

    [[nodiscard]] static traact::pattern::Pattern::Ptr GetBasePattern(const std::string& serializer_name) {

        std::string pattern_name = fmt::format("FileWriter_{0}_{1}", serializer_name, T::NativeTypeName);

        traact::pattern::Pattern::Ptr
            pattern =
            std::make_shared<traact::pattern::Pattern>(pattern_name, Concurrency::SERIAL, ComponentType::SYNC_SINK);

        pattern->addConsumerPort("input", T::NativeTypeName);
        pattern->addStringParameter("File", "file.json");

        pattern->addCoordinateSystem("A").addCoordinateSystem("B").addEdge("A", "B", "input");

        return pattern;
    }

    bool configure(const pattern::instance::PatternInstance &pattern_instance, buffer::ComponentBufferConfig *data) override {
        bool result = pattern::setValueFromParameter(pattern_instance, "File", filename_, "");
        if (result) {
            openFile();
}
        return result;
    }

    bool stop() override {
        return closeFile();
    }

    bool processTimePoint(buffer::ComponentBuffer &data) override {

        const typename T::NativeType &input = data.getInput<T>(0);

        return saveValue(data.getTimestamp(), input);
    }

    virtual bool openFile() = 0;
    virtual bool closeFile() = 0;
    virtual bool saveValue(Timestamp timestamp, const typename T::NativeType &value) = 0;

 protected:
    std::string filename_;
    const std::string serializer_name_;


};

}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_FILEWRITER_H_
