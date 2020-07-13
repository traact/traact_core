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

#include "traact/buffer/TimeDomainManager.h"

namespace traact::buffer {

TimeDomainManager::TimeDomainManager(int time_domain, size_t
                                     ringbuffer_size,
                                     std::set<buffer::GenericFactoryObject::Ptr> generic_factory_objects
) : time_domain_(time_domain),
    ringbuffer_size_(
        ringbuffer_size), generic_factory_objects_(std::move(generic_factory_objects)), domain_timestamp_index_(0) {

}
int TimeDomainManager::requestBuffer(const TimestampType ts, const std::string &component_name) {

  while (true) {

    SPDLOG_TRACE("trying to get buffer component: {0}, ts: {1}", component_name, ts.time_since_epoch().count());
    // tests existing running buffers, concurrent access possible here
    {
      typename RunningBufferType::const_accessor findResult;
      if (running_buffers_.find(findResult, ts)) {
        SPDLOG_TRACE("buffer found in running buffers");
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
        SPDLOG_TRACE("in lock: buffer found in running buffers, component: {0}", component_name);
        findResult.release();
        return 0;
      }

      if (!free_buffers_.empty()) {
        TimeDomainBufferPtr freeBuffer;

        if (free_buffers_.try_pop(freeBuffer)) {
          SPDLOG_TRACE("in lock: new buffer for component: {0} ts: {1}", component_name, ts.time_since_epoch().count());
          freeBuffer->resetForTimestamp(ts, domain_timestamp_index_);
          running_buffers_.insert(std::make_pair(ts, freeBuffer));
          ++domain_timestamp_index_;
          return 0;
        }
      }
    }

    switch (source_mode_) {
      default:
      case SourceMode::WaitForBuffer: {
        SPDLOG_TRACE("no free buffer, yield");
        std::this_thread::yield();
        //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        break;
      }
      case SourceMode::ImmediateReturn: {
        SPDLOG_TRACE("no free buffer, return");
        return -1;
      }

    }
  }
};

TimeDomainManager::DefaultComponentBuffer &TimeDomainManager::acquireBuffer(const TimestampType ts,
                                                                            const std::string &component_name) {
  typename RunningBufferType::const_accessor findResult;
  if (running_buffers_.find(findResult, ts)) {
    std::shared_ptr<DefaultTimeDomainBuffer> foundBuffer = findResult->second;
    findResult.release();

    return foundBuffer->getComponentBuffer(component_name);

  }
  throw std::invalid_argument("no domain buffer found for timestamp");
};

int TimeDomainManager::commitBuffer(TimestampType ts) {

  typename RunningBufferType::const_accessor findResult;
  SPDLOG_TRACE("commitBuffer ts: {0}",ts.time_since_epoch().count());
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

void TimeDomainManager::init(const ComponentGraphPtr &component_graph) {

  for (size_t index = 0; index < ringbuffer_size_; ++index) {
    free_buffers_.push(std::make_shared<DefaultTimeDomainBuffer>(time_domain_,component_graph, generic_factory_objects_));
  }

};
SourceMode TimeDomainManager::getSourceMode() const {
  return source_mode_;
}
void TimeDomainManager::setSourceMode(SourceMode source_mode) {
  source_mode_ = source_mode;
}
size_t TimeDomainManager::GetDomainMeasurementIndex(TimestampType ts) {
  typename RunningBufferType::const_accessor findResult;
  if (running_buffers_.find(findResult, ts)) {
    std::shared_ptr<DefaultTimeDomainBuffer> foundBuffer = findResult->second;
    findResult.release();

    return foundBuffer->GetCurrentMeasurementIndex();

  }
  throw std::invalid_argument("no domain buffer found for timestamp");
}

}