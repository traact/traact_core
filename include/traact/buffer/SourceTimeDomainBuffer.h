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

#ifndef TRAACTMULTI_SOURCETIMEDOMAINBUFFER_H
#define TRAACTMULTI_SOURCETIMEDOMAINBUFFER_H

#include <atomic>

#include <traact/component/ComponentGraph.h>
#include <traact/buffer/BufferFactory.h>
#include <traact/traact_core_export.h>
#include "BufferTypeConversion.h"


namespace traact::buffer {
    class TimeDomainManager;

    class TRAACT_CORE_EXPORT SourceTimeDomainBuffer {
    public:
        SourceTimeDomainBuffer(std::vector<void*> bufferData, TimeDomainManager *tdManager,
                                      bool isMaster, size_t componentIndex, std::size_t source_td_buffer_index,
                                      std::vector<size_t> global_buffer_index);

        void Init(traact::TimestampType ts, std::size_t mea_idx);
        bool Commit(bool valid = true);

        template<typename ReturnType, typename HeaderType>
        ReturnType &getOutput(size_t index) {
            return type_conversion_.asMutable<ReturnType, HeaderType>(buffer_data_[index], 0);
        }

        const TimestampType &GetTs() const;

        bool IsMaster() const;

        /**
         * index of component in component list of time domain manager
         * @return
         */
        size_t GetComponentIndex() const;

        /**
         * index of this source time domain buffer in a time domain buffer
         * indexes start at 0, continuous
         * @return
         */
        size_t GetSourceTDBufferIndex() const;

        void** GetBufferData();
        /**
         * list of index that map the local buffer index (0..n) to global buffer index in time domain manager
         * @return
         */
        const std::vector<size_t>& GetGlobalBufferIndex();

        MessageType GetMessageType();
        void SetMessageType(MessageType  msg);

        std::size_t GetMeaIdx();

    private:
        std::vector<void*> buffer_data_;
        std::vector<size_t> global_buffer_index_;
        BufferTypeConversion type_conversion_;
        TimeDomainManager* td_manager_;
        TimestampType  ts_;
        std::size_t mea_idx_;
        bool is_master_;
        std::size_t component_index_;
        std::size_t source_td_buffer_index_;
        MessageType msg_type_;
    };
}



#endif //TRAACTMULTI_SOURCETIMEDOMAINBUFFER_H
