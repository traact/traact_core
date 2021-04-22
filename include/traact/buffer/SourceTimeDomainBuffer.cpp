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

#include "SourceTimeDomainBuffer.h"
#include "TimeDomainManager.h"

traact::buffer::SourceTimeDomainBuffer::SourceTimeDomainBuffer(std::vector<void*> bufferData,
                                                                             TimeDomainManager *tdManager,
                                                                             bool isMaster, size_t componentIndex,
                                                                             std::size_t source_td_buffer_index,
                                                                             std::vector<size_t> global_buffer_index)
        : buffer_data_(std::move(bufferData)), td_manager_(tdManager), is_master_(isMaster), global_buffer_index_(std::move(global_buffer_index)),
          component_index_(componentIndex), source_td_buffer_index_(source_td_buffer_index) {
}

const traact::TimestampType &traact::buffer::SourceTimeDomainBuffer::GetTs() const {
    return ts_;
}

bool traact::buffer::SourceTimeDomainBuffer::IsMaster() const {
    return is_master_;
}

size_t traact::buffer::SourceTimeDomainBuffer::GetComponentIndex() const {
    return component_index_;
}

void traact::buffer::SourceTimeDomainBuffer::Init(traact::TimestampType ts, std::size_t mea_idx) {
    ts_ = ts;
    mea_idx_ = mea_idx;
}

bool traact::buffer::SourceTimeDomainBuffer::Commit(bool valid) {
    return td_manager_->CommitSourceBuffer(this, valid);
}

size_t traact::buffer::SourceTimeDomainBuffer::GetSourceTDBufferIndex() const {
    return source_td_buffer_index_;
}

void **traact::buffer::SourceTimeDomainBuffer::GetBufferData()  {
    return &buffer_data_[0];
}

const std::vector<size_t> &traact::buffer::SourceTimeDomainBuffer::GetGlobalBufferIndex() {
    return global_buffer_index_;
}

traact::MessageType traact::buffer::SourceTimeDomainBuffer::GetMessageType() {
    return msg_type_;
}

void traact::buffer::SourceTimeDomainBuffer::SetMessageType(traact::MessageType msg) {
    msg_type_ = msg;
}

std::size_t traact::buffer::SourceTimeDomainBuffer::GetMeaIdx() {
    return mea_idx_;
}
