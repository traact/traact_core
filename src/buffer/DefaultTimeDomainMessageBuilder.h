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

#ifndef TRAACTMULTI_DEFAULTTIMEDOMAINMESSAGEBUILDER_H
#define TRAACTMULTI_DEFAULTTIMEDOMAINMESSAGEBUILDER_H

#include <traact/buffer/TimeDomainMessageBuilder.h>

namespace traact::buffer {
    class DefaultTimeDomainMessageBuilder : public TimeDomainMessageBuilder{
    public:
        DefaultTimeDomainMessageBuilder(const TimeDomainManagerConfig &config,
                                        const std::map<std::string, size_t> &nameToBufferSource,
                                        const std::vector<GenericTimeDomainBuffer *> &allTdBuffer,
                                        GenericTimeDomainBuffer *configureMessageTdBuffer,
                                        const std::vector<std::vector<GenericSourceTimeDomainBuffer *>> &allSourceBuffer);

        GenericSourceTimeDomainBuffer *
        RequestSourceBuffer(const TimestampType ts, const std::string &component_name) override;

        GenericSourceTimeDomainBuffer *RequestNextSourceBuffer(const std::string &component_name) override;

        bool CommitSourceBuffer(GenericSourceTimeDomainBuffer *buffer, bool valid) override;

        void ReleaseTimeDomainBuffer(GenericTimeDomainBuffer *td_buffer) override;

        void Stop() override;

        void Configure() override;

        void Start() override;

        void Teardown() override;

    private:
        bool InternalCommitBuffer(GenericSourceTimeDomainBuffer *buffer);
        void InvalidateOlderTD(std::size_t mea_idx, std::size_t source_idx);

        bool initialized_{false};
        typedef tbb::recursive_mutex TD_Lock;


        std::map<std::size_t, GenericTimeDomainBuffer*> td_ringbuffer_;

        std::vector<std::size_t> source_mea_idx_;

        std::vector<tbb::concurrent_queue<GenericSourceTimeDomainBuffer*> > free_source_buffer_;
        std::vector< std::shared_ptr<Semaphore> > free_source_buffer_lock_;
        //std::vector<tbb::concurrent_queue<GenericSourceTimeDomainBuffer*> > unassigned_source_buffer_;
        std::vector<std::list<GenericSourceTimeDomainBuffer*> > unassigned_source_buffer_;

        TD_Lock buffer_mutex_;

        std::atomic<bool> index_calc_init_{false};
        WaitForInit wait_for_index_index_lock;
        std::vector<TimestampType> latest_ts_;
        std::size_t GetMinRunningIndex();
    };

}




#endif //TRAACTMULTI_DEFAULTTIMEDOMAINMESSAGEBUILDER_H
