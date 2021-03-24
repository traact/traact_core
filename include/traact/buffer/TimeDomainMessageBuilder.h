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

#ifndef TRAACTMULTI_TIMEDOMAINMESSAGEBUILDER_H
#define TRAACTMULTI_TIMEDOMAINMESSAGEBUILDER_H

#include <traact/datatypes.h>
#include <traact/buffer/GenericTimeDomainBuffer.h>
#include <traact/buffer/GenericComponentBuffer.h>
#include <traact/buffer/BufferSource.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>
#include <mutex>
#include <shared_mutex>
#include <traact/util/Logging.h>
#include <thread>
#include <traact/traact_core_export.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/queuing_rw_mutex.h>
#include <tbb/spin_rw_mutex.h>
#include <tbb/recursive_mutex.h>
#include <traact/util/Semaphore.h>
#include <list>
#include "GenericSourceTimeDomainBuffer.h"

namespace traact::component {
    class ComponentGraph;
}

namespace traact::buffer {


    class TRAACT_CORE_EXPORT TimeDomainMessageBuilder {
    public:

        TimeDomainMessageBuilder(TimeDomainManagerConfig config,
                                 std::map<std::string, size_t> nameToBufferSource,
                                 std::vector<GenericTimeDomainBuffer *> allTdBuffer,
                                 GenericTimeDomainBuffer *configureMessageTdBuffer,
                                 std::vector<std::vector<GenericSourceTimeDomainBuffer *>> allSourceBuffer);

        virtual GenericSourceTimeDomainBuffer *RequestSourceBuffer(const TimestampType ts, const std::string &component_name) = 0;

        virtual bool CommitSourceBuffer(GenericSourceTimeDomainBuffer *buffer, bool valid) =0 ;

        virtual void ReleaseTimeDomainBuffer(GenericTimeDomainBuffer *td_buffer) = 0;


        virtual void Configure() = 0;
        virtual void Start() = 0;
        virtual void Stop() = 0;
        virtual void Teardown() = 0;


    protected:
        bool IsTimestampMatch(TimestampType master, TimestampType subordinate);
        bool GetMeaIdx(TimestampType ts, std::size_t &meaIdx);
        std::size_t GetMaxMeaIdx();
        TimestampType  GetMeaIdxTs(std::size_t meaIdx);

        TimeDomainManagerConfig config_;
        TimestampType first_master_ts_{std::chrono::nanoseconds (0)};
        std::size_t current_mea_idx_{0};
        std::size_t next_mea_idx_{0};

        // data
        std::map<std::string, size_t> name_to_buffer_source_;
        std::vector<GenericTimeDomainBuffer*> all_td_buffer_;
        GenericTimeDomainBuffer* configure_message_td_buffer_;
        std::vector<std::vector<GenericSourceTimeDomainBuffer*> > all_source_buffer_;
    };
}

#endif //TRAACTMULTI_TIMEDOMAINMESSAGEBUILDER_H
