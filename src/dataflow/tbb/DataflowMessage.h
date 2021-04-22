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

#ifndef TRAACTMULTI_DATAFLOWMESSAGE_H
#define TRAACTMULTI_DATAFLOWMESSAGE_H

#include <traact/datatypes.h>
#include <traact/buffer/TimeDomainBuffer.h>
#include <tbb/tbb_stddef.h>

namespace traact::dataflow {


    struct TraactMessage {
            MessageType message_type = MessageType::Invalid;
            std::size_t event_idx = 0;
            buffer::TimeDomainBuffer* domain_buffer = nullptr;
            bool valid_data = false;



            uint64_t key() const {
                return event_idx;
            }

            inline void merge(const TraactMessage& msg){
                __TBB_ASSERT(event_idx == msg.event_idx, "event index of sync input differ");
                __TBB_ASSERT(domain_buffer == msg.domain_buffer, "domain_buffer of sync input differ");

                valid_data = valid_data && msg.valid_data;

                switch (msg.message_type) {
                    case MessageType::Invalid: {
                        message_type = MessageType::Invalid;
                        break;
                    }
                    default:
                        break;

                }
            }

            std::string toString() const {
                std::stringstream ss;
                ss << "TraactMessage MeaIndex: " << event_idx << " message type: " << static_cast<int>(message_type) << std::endl;
                return ss.str();
            }

        bool operator==(const TraactMessage &rhs) const {
            return message_type == rhs.message_type &&
                   event_idx == rhs.event_idx &&
                   domain_buffer == rhs.domain_buffer &&
                   valid_data == rhs.valid_data;
        }

        bool operator!=(const TraactMessage &rhs) const {
            return !(rhs == *this);
        }
    };
}

#endif //TRAACTMULTI_DATAFLOWMESSAGE_H
