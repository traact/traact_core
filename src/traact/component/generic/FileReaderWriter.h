/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_FILEREADERWRITER_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_FILEREADERWRITER_H_

#include "traact/component/ModuleComponent.h"
#include "traact/component/Component.h"
#include "traact/traact_plugins.h"
#include <spdlog/spdlog.h>
#include <traact/pattern/ParameterUtils.h>
#include <traact/util/FileUtil.h>
#include <traact/buffer/ComponentBuffer.h>

namespace traact::component {

template<typename T>
class FileReaderWriterModule : public Module {

 public:

    std::optional<typename T::NativeType> value_{};

};

template<typename T>
class FileReaderWriterModuleComponent : public ModuleComponent {
 public:
    explicit FileReaderWriterModuleComponent(const std::string &name) : ModuleComponent(name,
                                                                                        ModuleType::UNIQUE_DATAFLOW_PARAMETER) {}

    std::string getModuleKey() override {
        return filename_;
    }
    Module::Ptr instantiateModule() override {
        return std::make_shared<FileReaderWriterModule<T>>();
    }
    void configureInstance(const pattern::instance::PatternInstance &pattern_instance) override {
        pattern_instance.template setValueFromParameter("file", filename_);
        Component::configureInstance(pattern_instance);
    }
    bool configure(const pattern::instance::PatternInstance &pattern_instance,
                   buffer::ComponentBufferConfig *data) override {
        value_module_ = std::dynamic_pointer_cast<FileReaderWriterModule<T>>(module_);
        return true;
    }

 protected:
    std::string filename_;
    std::shared_ptr<FileReaderWriterModule<T>> value_module_;

};

template<typename T>
class FileReaderWriterRead : public FileReaderWriterModuleComponent<T> {
 public:
    explicit FileReaderWriterRead(const std::string &name, std::string serializer_name)
        : FileReaderWriterModuleComponent<T>(name),
          serializer_name_(std::move(serializer_name)) {
    }

    [[nodiscard]] static traact::pattern::Pattern::Ptr GetBasePattern(const std::string &serializer_name) {

        std::string pattern_name = fmt::format("FileReaderWriterRead_{0}_{1}", serializer_name, T::NativeTypeName);

        traact::pattern::Pattern::Ptr
            pattern =
            std::make_shared<traact::pattern::Pattern>(pattern_name, Concurrency::SERIAL, ComponentType::SYNC_SOURCE);

        pattern->addProducerPort("output", T::NativeTypeName);
        pattern->addStringParameter("file", "file.json");

        pattern->addCoordinateSystem("A").addCoordinateSystem("B").addEdge("A", "B", "output");

        return pattern;
    }

    bool configure(const pattern::instance::PatternInstance &pattern_instance,
                   buffer::ComponentBufferConfig *data) override {
        FileReaderWriterModuleComponent<T>::configure(pattern_instance, data);

        try {
            if (util::fileExists(FileReaderWriterModuleComponent<T>::filename_, Component::getName())) {
                typename T::NativeType file_value;
                if (readValue(file_value)) {
                    FileReaderWriterModuleComponent<T>::value_module_->value_ = {file_value};
                }

            }
        } catch (std::exception &e) {
            SPDLOG_ERROR(e.what());
        }

        return true;
    }

    virtual bool readValue(typename T::NativeType &data) = 0;

    bool processTimePoint(buffer::ComponentBuffer &data) override {
        auto &output = data.template getOutput<T>(0);
        if (FileReaderWriterModuleComponent<T>::value_module_->value_) {
            output = FileReaderWriterModuleComponent<T>::value_module_->value_.value();
        }

        return true;
    }

 protected:
    const std::string serializer_name_;
};

template<typename T>
class FileReaderWriterWrite : public FileReaderWriterModuleComponent<T> {
 public:
    explicit FileReaderWriterWrite(const std::string &name, std::string serializer_name) :
        FileReaderWriterModuleComponent<T>(name),
        serializer_name_(std::move(serializer_name)) {
    }

    [[nodiscard]] static traact::pattern::Pattern::Ptr GetBasePattern(const std::string &serializer_name) {

        std::string pattern_name = fmt::format("FileReaderWriterWrite_{0}_{1}", serializer_name, T::NativeTypeName);

        traact::pattern::Pattern::Ptr
            pattern =
            std::make_shared<traact::pattern::Pattern>(pattern_name, Concurrency::SERIAL, ComponentType::SYNC_SINK);

        pattern->addConsumerPort("input", T::NativeTypeName);
        pattern->addStringParameter("file", "file.json");

        pattern->addCoordinateSystem("A").addCoordinateSystem("B").addEdge("A", "B", "input");

        return pattern;
    }

    bool processTimePoint(buffer::ComponentBuffer &data) override {

        const typename T::NativeType &input = data.getInput<T>(0);

        FileReaderWriterModuleComponent<T>::value_module_->value_ = {input};


        return saveValue(data.getTimestamp(), input);
    }

    virtual bool saveValue(Timestamp timestamp, const typename T::NativeType &value) = 0;

 protected:
    const std::string serializer_name_;

};

} // traact

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_GENERIC_FILEREADERWRITER_H_
