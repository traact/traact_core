/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_BUFFER_BUFFERUTILS_H_
#define TRAACT_CORE_SRC_TRAACT_BUFFER_BUFFERUTILS_H_

#include <traact/datatypes.h>
#include <vector>

namespace traact::buffer {

enum class PortState : bool {
    INVALID = false,
    VALID = true
} ;

//using PortStateShared = std::atomic<PortState>;
using PortStateShared = PortState;

using LocalDataBuffer = std::vector<void *>;
using LocalHeaderBuffer = std::vector<void *>;
using LocalTimestampBuffer = std::vector<Timestamp *>;
using LocalValidBuffer = std::vector<PortStateShared *>;
using DataTypeNameVector = std::vector<std::string>;
}

#endif //TRAACT_CORE_SRC_TRAACT_BUFFER_BUFFERUTILS_H_
