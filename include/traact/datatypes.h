/*  BSD 3-Clause License
 *
 *  Copyright (c) 2020, FriederPankratz <frieder.pankratz@gmail.com>
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#ifndef TRAACT_INCLUDE_TRAACT_DATATYPES_H_
#define TRAACT_INCLUDE_TRAACT_DATATYPES_H_

#include <chrono>
#include <sstream>
#include <traact/traact_core_export.h>
namespace traact {

typedef std::chrono::duration<uint64_t, std::nano> TimeDurationType;
typedef typename std::chrono::time_point<std::chrono::system_clock, TimeDurationType> TimestampType;

struct TRAACT_CORE_EXPORT TimestampHashCompare {
  static size_t hash(const TimestampType &x) {
    return std::hash<size_t >{}(x.time_since_epoch().count());
  }
  static bool equal(const TimestampType &x, const TimestampType &y) {
    return x == y;
  }
};

inline static TimestampType now() {
  return std::chrono::system_clock::now();
}

enum class MessageDataMode {
  Invalid = 0,
  Static,
  Dynamic
};

enum class MessageType {
    Invalid = 0,
    Configure,
    Start,
    Data ,
    Stop,
    Teardown

};

enum class SourceMode {
  WaitForBuffer = 0,
  ImmediateReturn
};

enum class MissingSourceEventMode {
    WaitForEvent = 0,
    CancelOldest
};

// redefine tbb concurrency to avoid dependency in components just for this enum
enum concurrency { unlimited = 0, serial = 1 };

namespace buffer {
    template<typename HeaderType, std::size_t PortValue> class PortConfig {
    public:
        using Header = HeaderType;
        enum { PortIdx = PortValue };
    };

    struct TRAACT_CORE_EXPORT TimeDomainManagerConfig {
        int time_domain{-1};
        std::size_t ringbuffer_size{0};
        TimeDurationType max_offset{TimeDurationType::min()};
        TimeDurationType  max_delay{TimeDurationType::min()};
        TimeDurationType measurement_delta{TimeDurationType::min()};
        SourceMode source_mode{SourceMode::WaitForBuffer};
        MissingSourceEventMode missing_source_event_mode{MissingSourceEventMode::WaitForEvent};
        std::string master_source{"invalid"};

    };
}

}

#endif //TRAACT_INCLUDE_TRAACT_DATATYPES_H_
