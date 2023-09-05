/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_BUFFER_DATABUFFERFACTORY_H_
#define TRAACT_CORE_SRC_TRAACT_BUFFER_DATABUFFERFACTORY_H_

#include "DataBuffer.h"

namespace traact::buffer {
class DataBufferFactory {
 public:
    virtual ~DataBufferFactory() = default;
    virtual DataBufferPtr createBuffer() = 0;

};

using DataBufferFactoryPtr = std::shared_ptr<DataBufferFactory>;
}

#endif //TRAACT_CORE_SRC_TRAACT_BUFFER_DATABUFFERFACTORY_H_
