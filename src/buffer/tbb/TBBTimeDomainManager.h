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

#ifndef TRAACTMULTI_TBBTIMEDOMAINMANAGER_H
#define TRAACTMULTI_TBBTIMEDOMAINMANAGER_H

#include_next <tbb/flow_graph.h>

#include <traact/buffer/TimeDomainManager.h>
#include <dataflow/tbb/SourceMergeNode.h>
#include <traact/util/Semaphore.h>
#include <dataflow/tbb/DynamicJoinNode.h>
#include <dataflow/tbb/DataflowMessage.h>
#include <tbb/concurrent_queue.h>

namespace traact::dataflow {
    class NetworkGraph;
}


namespace traact::buffer {



    class TBBTimeDomainManager  : public TimeDomainManager{


    public:
        TBBTimeDomainManager(const TimeDomainManagerConfig &config,
                             const std::set<buffer::BufferFactory::Ptr> &genericFactoryObjects,
                             dataflow::NetworkGraph *graph);

        void Init(const ComponentGraphPtr &component_graph) override;

        SourceTimeDomainBuffer *
        RequestSourceBuffer(const TimestampType ts, const std::string &component_name) override;



        bool CommitSourceBuffer(SourceTimeDomainBuffer *buffer, bool valid) override;

        void ReleaseTimeDomainBuffer(TimeDomainBuffer *td_buffer) override;

        void Configure() override;

        void Start() override;

        void Stop() override;

        void Teardown() override;

    private:

        typedef typename dataflow::DynamicJoinNode JoinNode;
        typedef typename tbb::flow::function_node<dataflow::TraactMessage> FunctionNode;

        traact::dataflow::NetworkGraph* graph_;

        typedef tbb::spin_mutex TD_Lock;
        TD_Lock event_idx_lock_;
        std::size_t next_event_idx_{0};
        std::size_t next_td_buffer_event_idx_{0};

        TimestampType first_master_ts_{std::chrono::nanoseconds (0)};
        std::size_t non_data_events_{0};

        std::vector<dataflow::SourceMergeNode::Ptr > all_source_nodes_;
        std::vector<dataflow::SourceMergeNode::Ptr > async_source_nodes_;
        std::vector<dataflow::SourceMergeNode::Ptr > sync_source_nodes_;

        std::vector<std::size_t> latest_source_event_idx_;

        std::vector<TimestampType> latest_ts_;
        std::atomic<bool> index_calc_init_{false};
        std::atomic<bool> running_{false};
        WaitForInit wait_for_init_lock;
        WaitForInit wait_for_configure_finished_lock;
        WaitForInit wait_for_start_finished_lock;
        WaitForInit wait_for_stop_finished_lock;
        WaitForInit wait_for_teardown_finished_lock;

        tbb::concurrent_queue<TimeDomainBuffer*> free_time_domain_buffer_;
        std::vector<tbb::concurrent_queue<SourceTimeDomainBuffer*> > free_source_buffer_;
        std::vector< std::shared_ptr<Semaphore> > free_source_buffer_lock_;

        std::shared_ptr<JoinNode> sink_join_node_;
        std::shared_ptr<FunctionNode> emit_td_node_;

        void FinishEvent(const dataflow::TraactMessage &in);

        bool GetMeaIdx(TimestampType ts, std::size_t &meaIdx);
        TimestampType GetMeaIdxTs(std::size_t meaIdx);
        bool IsTimestampMatch(traact::TimestampType master, traact::TimestampType subordinate);
        std::size_t GetNextEventForNonData();


        SourceTimeDomainBuffer *
        RequestSourceBuffer(const TimestampType ts, const std::size_t event_index, const std::size_t source_index);

        void EmitNonDataEvent(MessageType message_type);
        void EmitEvent(const dataflow::SourceEventMessage& msg );
        void EmitEvent(TimeDomainBuffer* td_buffer );
        void CancelOlderEvents(std::size_t source_index, std::size_t current_event);

        void EmitSyncSourceEvent(std::size_t event_idx, TimestampType ts);
    };


}


#endif //TRAACTMULTI_TBBTIMEDOMAINMANAGER_H

