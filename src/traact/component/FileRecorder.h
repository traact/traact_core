/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

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

    traact::pattern::Pattern::Ptr GetPattern() const {

        std::string pattern_name = fmt::format("FileRecorder_{0}_{1}", FileWriter<T>::serializer_name_, T::MetaType);

        traact::pattern::Pattern::Ptr
            pattern =
            std::make_shared<traact::pattern::Pattern>(pattern_name, Concurrency::SERIAL);

        pattern->addConsumerPort("input", T::MetaType);

        pattern->addStringParameter("file", "file.json");

        pattern->addCoordinateSystem("A")
            .addCoordinateSystem("B")
            .addEdge("A", "B", "input");

        return pattern;
    }

 RTTR_ENABLE(Component)

};

}

#endif //TRAACT_CORE_SRC_TRAACT_COMPONENT_FILERECORDER_H_
