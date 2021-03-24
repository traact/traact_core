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

#include <traact/buffer/TimeDomainMessageBuilder.h>

#include <utility>

namespace traact::buffer {

    bool TimeDomainMessageBuilder::GetMeaIdx(TimestampType ts, size_t &meaIdx) {
        TimeDurationType timeDiff = ts - first_master_ts_;
        double tmp = static_cast<double>(timeDiff.count()) / static_cast<double>(config_.measurement_delta.count());
        // +1 for 0 is initial Init
        meaIdx = std::round(tmp)+1;

        TimestampType meaTs = GetMeaIdxTs(meaIdx);

        if(IsTimestampMatch(meaTs, ts)){
            int64_t tmp = static_cast<int64_t>(ts.time_since_epoch().count()) - static_cast<int64_t>(meaTs.time_since_epoch().count());
            SPDLOG_TRACE("timestamp match with offset {0}", tmp);
            return true;
        } else {
            return false;
        }
    }

    std::size_t TimeDomainMessageBuilder::GetMaxMeaIdx() {
        return next_mea_idx_ - 1;
    }

    TimestampType TimeDomainMessageBuilder::GetMeaIdxTs(std::size_t meaIdx) {
        return first_master_ts_ + config_.measurement_delta*(meaIdx-1);
    }

    bool TimeDomainMessageBuilder::IsTimestampMatch(TimestampType master, TimestampType subordinate) {
        //TODO need check since ts 1ns is used for Init ts, don't like this
        if(master == subordinate)
            return true;
        if(master.time_since_epoch().count() > 1)
            return ((master-config_.max_offset) < subordinate && (master+config_.max_offset) > subordinate);

        return false;
    }



    TimeDomainMessageBuilder::TimeDomainMessageBuilder(TimeDomainManagerConfig config,
                                                       std::map<std::string, size_t> nameToBufferSource,
                                                       std::vector<GenericTimeDomainBuffer *> allTdBuffer,
                                                       GenericTimeDomainBuffer *configureMessageTdBuffer,
                                                       std::vector<std::vector<GenericSourceTimeDomainBuffer *>> allSourceBuffer)
            : config_(std::move(config)), name_to_buffer_source_(std::move(nameToBufferSource)), all_td_buffer_(std::move(allTdBuffer)),
              configure_message_td_buffer_(configureMessageTdBuffer), all_source_buffer_(std::move(allSourceBuffer)) {}
}