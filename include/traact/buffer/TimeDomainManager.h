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
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_queue.h>
#include <mutex>
#include <shared_mutex>
#include <spdlog/spdlog.h>
#include <thread>

namespace traact::component {
class ComponentGraph;
}

namespace traact::buffer {

//

class TimeDomainManager {
 public:
  typedef typename std::shared_ptr<TimeDomainManager> Ptr;
  typedef GenericTimeDomainBuffer DefaultTimeDomainBuffer;
  typedef typename GenericTimeDomainBuffer::ComponentBuffer DefaultComponentBuffer;

  typedef typename component::ComponentGraph ComponentGraph;
  typedef typename component::ComponentGraph::Ptr ComponentGraphPtr;
  typedef typename std::shared_ptr<GenericTimeDomainBuffer> TimeDomainBufferPtr;
  typedef typename tbb::concurrent_hash_map<TimestampType, TimeDomainBufferPtr, TimestampHashCompare> RunningBufferType;

  TimeDomainManager(size_t ringbuffer_size,
                    std::set<buffer::GenericFactoryObject::Ptr> generic_factory_objects) : ringbuffer_size_(
      ringbuffer_size), generic_factory_objects_(std::move(generic_factory_objects)) {

  }
  int requestBuffer(const TimestampType ts, const std::string &component_name) {

    while (true) {

      spdlog::trace("trying to get buffer component: {0}, ts: {1}", component_name, ts.time_since_epoch().count());
      // test existing running buffers, concurrent access possible here
      {
        typename RunningBufferType::const_accessor findResult;
        if (running_buffers_.find(findResult, ts)) {
          findResult.release();
          return 0;
        }
      }

      // check free buffers, must be made exclusive for now
      {
        std::unique_lock lock(mutex_);
        // check map again because a different thread might have also waited for the lock and created the buffer
        typename RunningBufferType::const_accessor findResult;
        if (running_buffers_.find(findResult, ts)) {
          findResult.release();
          return 0;
        }

        if (!free_buffers_.empty()) {
          TimeDomainBufferPtr freeBuffer;

          if (free_buffers_.try_pop(freeBuffer)) {
            freeBuffer->resetForTimestamp(ts);
            running_buffers_.insert(std::make_pair(ts, freeBuffer));
            return 0;
          }
        }
      }

      switch (source_mode_) {
        case SourceMode::WaitForBuffer: {
          std::this_thread::yield();
          break;
        }
        case SourceMode::ImmediateReturn: {
          return -1;
        }

      }

      /*spdlog::debug("not buffer found for ts:{0}", ts.time_since_epoch().count());
      spdlog::debug("free buffer unsafe size {0} running buffers: ", free_buffers_.unsafe_size());
      int i = 0;
      for (const auto &running_buffer : running_buffers_) {

        spdlog::debug("buffer state index: {0}, ts: {1}, useCount: {2}",
                      i++,
                      running_buffer.first.time_since_epoch().count(),
                      running_buffer.second->getUseCount());
      }*/


      // if no buffer is found continue blocking the user thread and yield so othe threads have a chance
      // change with different strageties later

    }
  };

  DefaultComponentBuffer &acquireBuffer(const TimestampType ts, const std::string &component_name) {
    typename RunningBufferType::const_accessor findResult;
    if (running_buffers_.find(findResult, ts)) {
      std::shared_ptr<DefaultTimeDomainBuffer> foundBuffer = findResult->second;
      findResult.release();

      return foundBuffer->getComponentBuffer(component_name);

    }
    throw std::invalid_argument("no domain buffer found for timestamp");
  };

  int commitBuffer(TimestampType ts) {

    typename RunningBufferType::const_accessor findResult;
    if (running_buffers_.find(findResult, ts)) {
      //no lock required, atomic<int>
      findResult->second->decreaseUse();

      if (findResult->second->isFree()) {
        TimeDomainBufferPtr freeBuffer = findResult->second;
        {
          std::unique_lock lock(mutex_);
          running_buffers_.erase(findResult);
          free_buffers_.push(freeBuffer);
        }

      }

      findResult.release();
      return 0;

    }

    throw std::invalid_argument("no domain buffer found for timestamp");

  };

  void init(const ComponentGraphPtr &component_graph) {

    for (size_t index = 0; index < ringbuffer_size_; ++index) {
      free_buffers_.push(std::make_shared<DefaultTimeDomainBuffer>(component_graph, generic_factory_objects_));
    }

  };
  SourceMode getSourceMode() const {
    return source_mode_;
  }
  void setSourceMode(SourceMode source_mode) {
    source_mode_ = source_mode;
  }
 private:
  std::size_t ringbuffer_size_;
  SourceMode source_mode_;

  //std::mutex buffer_lock_;
  mutable std::shared_mutex mutex_;

  tbb::concurrent_queue<TimeDomainBufferPtr> free_buffers_;
  RunningBufferType running_buffers_;
  std::set<buffer::GenericFactoryObject::Ptr> generic_factory_objects_;

};

}
#endif //TRAACTTEST_INCLUDE_TRAACT_TIMEPOINTBUFFER_TIMEDOMAINMANAGER_H_
