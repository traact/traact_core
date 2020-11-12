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
#include <spdlog/spdlog.h>
#include <thread>
#include <traact/traact_core_export.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/queuing_rw_mutex.h>
#include <tbb/spin_rw_mutex.h>
#include <traact/util/Semaphore.h>
#include <list>

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


  TimeDomainManager(int time_domain, std::string master, TimeDurationType max_offset, size_t ringbuffer_size,
                    std::set<buffer::GenericFactoryObject::Ptr> generic_factory_objects);

  void registerBufferSource(BufferSource::Ptr buffer_source);

  TimestampType requestBuffer(const TimestampType ts, const std::string &component_name);

  GenericTimeDomainBuffer::Ptr acquireTimeDomainBuffer(const TimestampType ts);
  DefaultComponentBuffer &acquireBuffer(const TimestampType ts, const std::string &component_name);

  //int commitBuffer(TimestampType ts);

  int releaseBuffer(TimestampType ts);

  void init(const ComponentGraphPtr &component_graph);
  void stop();
  SourceMode getSourceMode() const;

  void setSourceMode(SourceMode source_mode);

  size_t GetDomainMeasurementIndex(TimestampType ts);

 private:


    //typedef typename tbb:concurrent_hash_map<TimestampType, TimeDomainBufferPtr, TimestampHashCompare> RunningBufferType;
    typedef typename std::map<TimestampType, TimeDomainBufferPtr> RunningBufferType;
    //typedef tbb::spin_rw_mutex TD_Lock;
    typedef tbb::queuing_rw_mutex TD_Lock;
    //typedef typename std::list<TimeDomainBufferPtr> RunningBufferType;
  int time_domain_;
    TD_Lock buffer_mutex_;
  std::size_t ringbuffer_size_;
  SourceMode source_mode_;
  std::vector<BufferSource::Ptr> buffer_sources_;
  std::string master_;
  TimeDurationType max_subordinate_offset_;

  WaitForMasterTs new_ts_wait_;
  int source_count_;
  std::atomic<int> source_requests_;


  //std::mutex buffer_lock_;
  mutable std::shared_mutex mutex_;
  size_t domain_timestamp_index_;
  TimestampType  domain_timestamp_;
  std::set<TimestampType> subordiante_wait_ts_;

  tbb::concurrent_queue<TimeDomainBufferPtr> free_buffers_;
  RunningBufferType running_buffers_;
  std::set<buffer::GenericFactoryObject::Ptr> generic_factory_objects_;

  bool check_for_blocked_buffer(const TimestampType ts,const std::string &component_name);

    TimestampType requestBuffer_Master(const TimestampType ts, const std::string &component_name);
    TimestampType requestBuffer_Subordinate(const TimestampType ts, const std::string &component_name);

};

}
#endif //TRAACTTEST_INCLUDE_TRAACT_TIMEPOINTBUFFER_TIMEDOMAINMANAGER_H_
