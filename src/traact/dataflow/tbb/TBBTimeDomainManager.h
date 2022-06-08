/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACTMULTI_TBBTIMEDOMAINMANAGER_H
#define TRAACTMULTI_TBBTIMEDOMAINMANAGER_H

#include_next <tbb/flow_graph.h>

#include <traact/buffer/TimeDomainBuffer.h>
#include "SourceMergeNode.h"
#include "traact/util/Semaphore.h"
#include "DynamicJoinNode.h"
#include "DataflowMessage.h"
#include <tbb/concurrent_queue.h>
#include <traact/traact.h>
namespace traact::dataflow {
class NetworkGraph;
}

namespace traact::dataflow {

class TBBTimeDomainManager {

 public:
    TBBTimeDomainManager(const buffer::TimeDomainManagerConfig &config,
                         const std::set<buffer::BufferFactory::Ptr> &genericFactoryObjects,
                         dataflow::NetworkGraph *graph);

    void Init(const DefaultComponentGraphPtr &component_graph);

    std::future<buffer::SourceComponentBuffer *> RequestSourceBuffer(TimestampType ts, int component_index);

//        void ReleaseTimeDomainBuffer(TimeDomainBuffer *td_buffer);
//
    void Configure();

    void Start();

    void Stop();

    void Teardown();

    int GetComponentIndex(std::string instance_id);

 private:

    typedef typename dataflow::DynamicJoinNode JoinNode;
    typedef typename tbb::flow::function_node<dataflow::TraactMessage> FunctionNode;

    traact::dataflow::NetworkGraph *graph_;
    traact::buffer::TimeDomainManagerConfig time_domain_config_;
    DefaultComponentGraphPtr component_graph_;
    typedef tbb::spin_mutex TD_Lock;
    TD_Lock event_idx_lock_;
    size_t next_event_idx_{0};
    size_t next_td_buffer_event_idx_{0};

    TimestampType first_master_ts_{std::chrono::nanoseconds(0)};
    size_t non_data_events_{0};

    std::vector<dataflow::SourceMergeNode::Ptr> all_source_nodes_;
    std::vector<dataflow::SourceMergeNode::Ptr> async_source_nodes_;
    std::vector<dataflow::SourceMergeNode::Ptr> sync_source_nodes_;

    std::vector<size_t> latest_source_event_idx_;

    std::vector<TimestampType> latest_ts_;
    std::atomic<bool> index_calc_init_{false};
    std::atomic<bool> running_{false};
    WaitForInit wait_for_init_lock;
    WaitForInit wait_for_configure_finished_lock;
    WaitForInit wait_for_start_finished_lock;
    WaitForInit wait_for_stop_finished_lock;
    WaitForInit wait_for_teardown_finished_lock;

    tbb::concurrent_queue<buffer::TimeStepBuffer *> free_time_domain_buffer_;
    //std::vector<tbb::concurrent_queue<SourceTimeDomainBuffer*> > free_source_buffer_;
    buffer::TimeDomainBuffer time_domain_buffer_;
    std::vector<std::shared_ptr<Semaphore> > free_source_buffer_lock_;

    std::shared_ptr<JoinNode> sink_join_node_;
    std::shared_ptr<FunctionNode> emit_td_node_;

    void FinishEvent(const dataflow::TraactMessage &in);

    bool GetMeaIdx(TimestampType ts, size_t &meaIdx);
    TimestampType GetMeaIdxTs(size_t meaIdx);
    bool IsTimestampMatch(traact::TimestampType master, traact::TimestampType subordinate);
    size_t GetNextEventForNonData();

    void EmitNonDataEvent(MessageType message_type);
    void EmitEvent(const dataflow::SourceEventMessage &msg);
    void EmitEvent(buffer::TimeStepBuffer *td_buffer);
    void CancelOlderEvents(size_t source_index, size_t current_event);

    void EmitSyncSourceEvent(size_t event_idx, TimestampType ts);
};

}

#endif //TRAACTMULTI_TBBTIMEDOMAINMANAGER_H

