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

#ifndef TRAACTMULTI_TRAACTDATAFLOWMESSAGE_H
#define TRAACTMULTI_TRAACTDATAFLOWMESSAGE_H

#include <traact/datatypes.h>

namespace traact::buffer {
    class TRAACT_CORE_EXPORT GenericTimeDomainBuffer;
}

namespace traact {
    enum class MessageType {
        Invalid = 0,
        Data ,
        Parameter,
        AbortTs
    };

    struct TraactMessage {
            MessageType message_type = MessageType::Invalid;
            TimestampType timestamp = TimestampType::min();
            //std::shared_ptr<buffer::GenericTimeDomainBuffer> domain_buffer = nullptr;
            buffer::GenericTimeDomainBuffer* domain_buffer = nullptr;
            bool valid = false;
            size_t domain_measurement_index = 0;



            uint64_t key() const {
                return timestamp.time_since_epoch().count();
            }

            std::string toString() const {
                std::stringstream ss;
                ss << "TraactMessage TS: " << timestamp.time_since_epoch().count() << " MeaIndex: " << domain_measurement_index << " valid: " << valid << " message type: " << static_cast<int>(message_type) << std::endl;
                return ss.str();
            }
    };
}

#endif //TRAACTMULTI_TRAACTDATAFLOWMESSAGE_H
