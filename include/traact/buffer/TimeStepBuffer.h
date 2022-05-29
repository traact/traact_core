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

#ifndef TRAACTMULTI_TIMESTEPBUFFER_H
#define TRAACTMULTI_TIMESTEPBUFFER_H

#include <tuple>
#include <vector>
#include <map>
#include "ComponentBuffer.h"
#include "SourceComponentBuffer.h"
#include <traact/component/ComponentTypes.h>

namespace traact::buffer{


    struct BufferConfig {
        component::ComponentType component_type;
        std::vector<std::pair<int, int>> buffer_to_port_inputs;
        std::vector<std::pair<int, int>> buffer_to_port_output;
    };
    using BufferType = std::vector<void*>;

    class TimeStepBuffer {
    public:

        TimeStepBuffer(BufferType bufferData, std::map<int, std::pair<BufferConfig, std::string>> buffer_config, const SourceComponentBuffer::CommitCallback& callback);
        std::size_t GetComponentIndex(const std::string &component_name);
        ComponentBuffer &GetComponentBuffer(std::size_t component_idx);
        ComponentBuffer &GetComponentBuffer(const std::string &component_name);
        SourceComponentBuffer *GetSourceComponentBuffer(std::size_t component_idx);
        std::future<bool> GetSourceLock(std::size_t component_idx);

        void ResetForTimestamp(TimestampType ts);
        TimestampType GetTimestamp();


    private:
        TimestampType current_ts_;
        std::map<std::string, std::size_t > component_buffer_to_index_;
        std::vector< ComponentBuffer > component_buffers_list_;
        std::vector< std::shared_ptr<SourceComponentBuffer> > source_buffer_list_;
        BufferType buffer_data_;

    };
}



#endif //TRAACTMULTI_TIMESTEPBUFFER_H
