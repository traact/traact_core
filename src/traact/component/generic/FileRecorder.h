/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_COMPONENT_FILERECORDER_H_
#define TRAACT_CORE_SRC_TRAACT_COMPONENT_FILERECORDER_H_

#include "FileWriter.h"

namespace traact::component {

template<class T>
class FileRecorder : public FileWriter<T> {
 public:
    explicit FileRecorder(const std::string &name, const std::string &serializer_name) : FileWriter<T>(name,
                                                                                                       serializer_name) {
    }

    static traact::pattern::Pattern::Ptr GetPattern() {

        std::string pattern_name = fmt::format("FileRecorder_{0}_{1}", FileWriter<T>::serializer_name_, T::MetaType);

        traact::pattern::Pattern::Ptr
            pattern =
            std::make_shared<traact::pattern::Pattern>(pattern_name, Concurrency::SERIAL, ComponentType::SYNC_SINK);

        pattern->addConsumerPort("input", T::MetaType);

        pattern->addStringParameter("file", "file.json");

        pattern->addCoordinateSystem("A")
            .addCoordinateSystem("B")
            .addEdge("A", "B", "input");

        return pattern;
    }



};

}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_FILERECORDER_H_
