/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_FILEWRITER_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_FILEWRITER_H_

#include <rttr/registration>
#include <utility>
#include "traact/traact.h"
#include <fmt/format.h>

namespace traact::component {

template<class T>
class FileWriter : public Component {
 public:
    explicit FileWriter(const std::string &name, std::string serializer_name) : Component(name,
                                                                                                 traact::component::ComponentType::SYNC_SINK),
                                                                                       serializer_name_(std::move(serializer_name)) {
    }

    [[nodiscard]] traact::pattern::Pattern::Ptr GetPattern() const override {

        std::string pattern_name = fmt::format("FileWriter_{0}_{1}", serializer_name_, T::MetaType);

        traact::pattern::Pattern::Ptr
            pattern =
            std::make_shared<traact::pattern::Pattern>(pattern_name, Concurrency::SERIAL);

        pattern->addConsumerPort("input", T::MetaType);
        pattern->addStringParameter("file", "file.json");

        pattern->addCoordinateSystem("A").addCoordinateSystem("B").addEdge("A", "B", "input");

        return pattern;
    }

    bool configure(const nlohmann::json &parameter, buffer::ComponentBufferConfig *data) override {
        bool result = pattern::setValueFromParameter(parameter, "file", filename_, "");
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
 RTTR_ENABLE(Component)

};

}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_FILEWRITER_H_
