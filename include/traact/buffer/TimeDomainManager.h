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

#ifndef TRAACTTEST_INCLUDE_TRAACT_TIMEPOINTBUFFER_TIMEDOMAINMANAGER_H_
#define TRAACTTEST_INCLUDE_TRAACT_TIMEPOINTBUFFER_TIMEDOMAINMANAGER_H_

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
class TRAACT_CORE_EXPORT ComponentGraph;
}

namespace traact::buffer {



class TRAACT_CORE_EXPORT TimeDomainManager {
 public:
  typedef typename std::shared_ptr<TimeDomainManager> Ptr;
  typedef GenericTimeDomainBuffer DefaultTimeDomainBuffer;
  typedef typename GenericTimeDomainBuffer::ComponentBuffer DefaultComponentBuffer;

  typedef typename component::ComponentGraph ComponentGraph;
  typedef typename component::ComponentGraph::Ptr ComponentGraphPtr;
  typedef typename std::shared_ptr<GenericTimeDomainBuffer> TimeDomainBufferPtr;


  TimeDomainManager(TimeDomainManagerConfig config,
                    std::set<buffer::GenericFactoryObject::Ptr> generic_factory_objects);

    virtual ~TimeDomainManager();

    void RegisterBufferSource(BufferSource::Ptr buffer_source);

  GenericSourceTimeDomainBuffer* RequestBuffer(const TimestampType ts, const std::string &component_name);
  
  void ReleaseTimeDomainBuffer(GenericTimeDomainBuffer* td_buffer);

  bool CommitSourceBuffer(GenericSourceTimeDomainBuffer* buffer);


  void init(const ComponentGraphPtr &component_graph);
  void stop();
  SourceMode getSourceMode() const;


 private:

    //typedef tbb::queuing_rw_mutex TD_Lock;
    bool initialized_{false};
    typedef tbb::recursive_mutex TD_Lock;

    TimeDomainManagerConfig config_;



    std::atomic<std::size_t> current_mea_idx;
    std::atomic<std::size_t> next_mea_idx_;
    std::map<std::string, buffer::GenericFactoryObject::Ptr> generic_factory_objects_;

    ComponentGraphPtr component_graph_;

    std::vector<ComponentGraph::PatternComponentPair> source_components_;
    std::vector<BufferSource*> buffer_sources_;
    std::map<std::string, size_t> name_to_buffer_source_;


    std::map<std::size_t, GenericTimeDomainBuffer*> td_ringbuffer_;
    std::vector<GenericTimeDomainBuffer*> td_ringbuffer_list_;
    GenericTimeDomainBuffer* configure_message_td_buffer_;
    std::shared_ptr<Semaphore> td_ringbuffer_lock_;


    std::vector<GenericSourceTimeDomainBuffer*> all_source_buffer_;
    std::vector<tbb::concurrent_queue<GenericSourceTimeDomainBuffer*> > free_source_buffer_;
    std::vector< std::shared_ptr<Semaphore> > free_source_buffer_lock_;
    //std::vector<tbb::concurrent_queue<GenericSourceTimeDomainBuffer*> > unassigned_source_buffer_;
    std::vector<std::list<GenericSourceTimeDomainBuffer*> > unassigned_source_buffer_;
    TD_Lock buffer_mutex_;


    std::vector<std::string> buffer_datatype_;
    // buffer_data[buffer_size_index][buffer_index]
    std::vector<std::vector<void*> > buffer_data_;
    std::vector<std::vector<void*> > buffer_header_;
    std::map<pattern::instance::ComponentID_PortName, int> port_to_bufferIndex_;


    bool IsTimestampMatch(TimestampType master, TimestampType subordiante);
    bool IsAfterTimestamp(TimestampType master, TimestampType subordinate);

    bool GetMeaIdx(TimestampType ts, std::size_t &meaIdx);
    std::size_t GetMaxMeaIdx();
    //int GetTDBufferIdx(std::size_t meaIdx);
    TimestampType  GetMeaIdxTs(std::size_t meaIdx);
    void AdvanceBuffers();

    bool index_calc_init_{false};
    WaitForInit wait_for_index_index_lock;
    std::size_t meaIdx_offset_{0};
    TimestampType first_master_ts_{std::chrono::nanoseconds (0)};
    std::vector<TimestampType> latest_ts_;

    bool InternalCommitBuffer(GenericSourceTimeDomainBuffer *buffer);
    void InvalidateOlderTD(std::size_t mea_idx, std::size_t source_idx);


};

}
#endif //TRAACTTEST_INCLUDE_TRAACT_TIMEPOINTBUFFER_TIMEDOMAINMANAGER_H_
