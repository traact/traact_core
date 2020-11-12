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

TimeDomainManager::TimeDomainManager(int time_domain, std::string master, TimeDurationType max_offset,
                                     size_t ringbuffer_size,
                                     std::set<buffer::GenericFactoryObject::Ptr> generic_factory_objects) : time_domain_(time_domain),
    ringbuffer_size_(
        ringbuffer_size), generic_factory_objects_(std::move(generic_factory_objects)), domain_timestamp_index_(0),
    master_(master),
    max_subordinate_offset_(max_offset){

    domain_timestamp_ = TimestampType::min();



}

TimestampType TimeDomainManager::requestBuffer(const TimestampType ts, const std::string &component_name) {

    if (master_ == component_name) {
        return requestBuffer_Master(ts, component_name);
    } else {
        return requestBuffer_Subordinate(ts, component_name);
    }
}

TimestampType TimeDomainManager::requestBuffer_Master(const TimestampType ts, const std::string &component_name) {
    SPDLOG_INFO("request buffer master for component {0} ts {1}", component_name, ts.time_since_epoch().count());
    TimestampType request_start = now();
    while(true){

        // check if free buffer is available
        if (!free_buffers_.empty()) {
            TimeDomainBufferPtr freeBuffer;

            if (free_buffers_.try_pop(freeBuffer)) {
                TD_Lock::scoped_lock lock(buffer_mutex_, true);
                SPDLOG_INFO("in lock: new buffer for component: {0} ts: {1}", component_name,
                             ts.time_since_epoch().count());
                freeBuffer->resetForTimestamp(ts, domain_timestamp_index_, buffer_sources_);
                freeBuffer->increaseSourceCount(component_name);
                ++domain_timestamp_index_;
                domain_timestamp_ = ts;
                running_buffers_.emplace(ts, freeBuffer);
                subordiante_wait_ts_.erase(ts);
                new_ts_wait_.notifyAll(ts);
                return ts;
            }
        }
        // check for missing source inputs with a 10ms timeout
        if((request_start+std::chrono::milliseconds(100)) < now())
        {
            SPDLOG_TRACE("cancel old buffer for time domain {0}", time_domain_);
            TD_Lock::scoped_lock lock(buffer_mutex_, true);

            TimestampType min_ts = TimestampType::max();
            GenericTimeDomainBuffer::Ptr min_buffer;
            for(const auto& it : running_buffers_) {
                auto& buffer = it.second;
                // if all inputs are set then it should not be canceled
                if(buffer->isSourcesSet())
                    continue;

                TimestampType buffer_ts = buffer->getTimestamp();
                if(buffer_ts < min_ts){
                    min_ts = buffer_ts;
                    min_buffer = buffer;
                }
            }

            if(min_ts != TimestampType::max()){
                SPDLOG_INFO("invalidate buffer for time domain {0} ts {1}", time_domain_, min_ts.time_since_epoch().count() );
                min_buffer->invalidateBuffer();
            }

        }

        // no free buffer
        std::this_thread::yield();

    }
}
TimestampType TimeDomainManager::requestBuffer_Subordinate(const TimestampType ts, const std::string &component_name){
    SPDLOG_INFO("request buffer subordinate for component {0} ts {1}", component_name, ts.time_since_epoch().count());

    while(true) {
        // check existing buffers
        {

            TD_Lock::scoped_lock lock(buffer_mutex_, true);



            // check all running buffer for match with timestamp
            // TODO list would be sorted, faster then iterate over everything
            for(const auto& it : running_buffers_) {
                TimestampType domain_ts = it.first;

                // if the buffer is older then the current ts and is missing this source then this path in the graph is invalid
                // TODO ts with 1ns is used for init, don't like this check
                if(((ts-max_subordinate_offset_) < domain_ts && domain_ts < (ts+max_subordinate_offset_)) || ts.time_since_epoch().count() == 1) {
                    it.second->increaseSourceCount(component_name);
                    subordiante_wait_ts_.erase(ts);
                    return domain_ts;
                }
                if(ts < domain_ts){
                    //SPDLOG_INFO("component {1} ts before domain, diff: {0}", (domain_ts-ts).count(), component_name);
                }else {
                    //SPDLOG_INFO("component {1} domain before ts, diff: {0}", (ts-domain_ts).count(), component_name);
                }
            }

            // if the ts for this source will not be created anymore return invalid
            if(ts < domain_timestamp_){
                return TimestampType::min();
            }
            subordiante_wait_ts_.emplace(ts);
        }

        // wait for new master ts
        //new_ts_wait_.wait(ts);
        std::this_thread::yield();

    }


    return ts;
}

//TimestampType TimeDomainManager::requestBuffer(const TimestampType ts, const std::string &component_name) {
//
//    TimestampType start_request = now();
//  while (true) {
//
//    SPDLOG_TRACE("trying to get buffer for component: {0}, ts: {1}", component_name, ts.time_since_epoch().count());
//    // tests existing running buffers, concurrent access possible here
//    /*
//    {
//        TD_Lock::scoped_lock lock(buffer_mutex_, true);
//      //typename RunningBufferType::const_accessor findResult;
//      //if (running_buffers_.find(findResult, ts)) {
//      const auto& findResult = running_buffers_.find(ts);
//        if (findResult != running_buffers_.end()) {
//        SPDLOG_TRACE("buffer found in running buffers");
//        findResult->second->increaseSourceCount(component_name);
//        //findResult.release();
//        return 0;
//      }
//    }*/
//
//    // check free buffers, must be made exclusive for now
//    {
//      //std::unique_lock lock(mutex_);
//        TD_Lock::scoped_lock lock(buffer_mutex_, true);
//      // check map again because a different thread might have also waited for the lock and created the buffer
//      //typename RunningBufferType::const_accessor findResult;
//      //if (running_buffers_.find(findResult, ts)) {
//      const auto& findResult = running_buffers_.find(ts);
//      if (findResult != running_buffers_.end()) {
//        SPDLOG_TRACE("in lock: buffer found in running buffers, component: {0}", component_name);
//        findResult->second->increaseSourceCount(component_name);
//        //findResult.release();
//        return TimestampType::min();
//      }
//
//        // cleanup older buffers
//        for(const auto& it : running_buffers_) {
//            auto& buffer = it.second;
//            // if all inputs are set then it should not be canceled
//            if(buffer->isSourcesSet())
//                continue;
//            // if the buffer is older then the current ts and is missing this source then this path in the graph is invalid
//            if(buffer->getTimestamp() < ts) {
//                if(!buffer->isSourceSet(component_name)){
//                    buffer->cancelSource(component_name);
//                }
//            }
//        }
//
//      if (!free_buffers_.empty()) {
//        TimeDomainBufferPtr freeBuffer;
//
//        if (free_buffers_.try_pop(freeBuffer)) {
//
//          SPDLOG_TRACE("in lock: new buffer for component: {0} ts: {1}", component_name, ts.time_since_epoch().count());
//          freeBuffer->resetForTimestamp(ts, domain_timestamp_index_, buffer_sources_);
//          freeBuffer->increaseSourceCount(component_name);
//          ++domain_timestamp_index_;
//
//            running_buffers_.emplace(ts, freeBuffer);
//          return TimestampType::min();
//        }
//      }
//    }
//
//    switch (source_mode_) {
//      default:
//      case SourceMode::WaitForBuffer: {
//          /*
//          if(check_for_blocked_buffer(ts, component_name)){
//              SPDLOG_TRACE("found blocked buffer");
//              //std::this_thread::yield();
//          } else {
//              // TODO add timeout to requestBuffer
//              SPDLOG_TRACE("no free buffer, yield");
//              std::this_thread::yield();
//              //std::this_thread::sleep_for(std::chrono::milliseconds(1));
//          }*/
//          //check_for_blocked_buffer(ts, component_name);
//          std::this_thread::yield();
//
//        break;
//      }
//      case SourceMode::ImmediateReturn: {
//        SPDLOG_TRACE("no free buffer, return");
//        return TimestampType::min();
//      }
//
//    }
//  }
//};



    bool TimeDomainManager::check_for_blocked_buffer(const TimestampType ts, const std::string &component_name){
        //return false;
        TD_Lock::scoped_lock lock(buffer_mutex_, true);



        // check all running buffers
        for(const auto& it : running_buffers_) {
            auto& component = it.second;
            // if all inputs are set then it should not be canceled
            if(component->isSourcesSet())
                continue;
            // if the buffer is older then the current ts and the input of the component is set
            // the other sources have been to slow to feed the input, cancel older ts
            if(component->getTimestamp() < ts) {
                if(component->isSourceSet(component_name)){
                    component->cancelSource(component_name);
                }
            }
        }


        return false;
    }

    GenericTimeDomainBuffer::Ptr TimeDomainManager::acquireTimeDomainBuffer(const TimestampType ts) {
        TD_Lock::scoped_lock lock(buffer_mutex_, false);
        //typename RunningBufferType::const_accessor findResult;
        //if (running_buffers_.find(findResult, ts)) {
        const auto &findResult = running_buffers_.find(ts);
        if (findResult != running_buffers_.end()) {
            std::shared_ptr<DefaultTimeDomainBuffer> foundBuffer = findResult->second;

            //findResult.release();

            return foundBuffer;

        }

        return nullptr;
    }
TimeDomainManager::DefaultComponentBuffer &TimeDomainManager::acquireBuffer(const TimestampType ts,
                                                                            const std::string &component_name) {
    TD_Lock::scoped_lock lock(buffer_mutex_, true);
  //typename RunningBufferType::const_accessor findResult;
  //if (running_buffers_.find(findResult, ts)) {
    const auto& findResult = running_buffers_.find(ts);
    if (findResult != running_buffers_.end()) {
    std::shared_ptr<DefaultTimeDomainBuffer> foundBuffer = findResult->second;

    //findResult.release();

    return foundBuffer->getComponentBuffer(component_name);

  }
  SPDLOG_ERROR("no domain buffer found for timestamp {0}", ts.time_since_epoch().count());
  throw std::invalid_argument("no domain buffer found for timestamp "+ std::to_string(ts.time_since_epoch().count()));
};


/*
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

};*/

int TimeDomainManager::releaseBuffer(TimestampType ts) {

        TD_Lock::scoped_lock lock(buffer_mutex_, true);

        //typename RunningBufferType::const_accessor findResult;
        SPDLOG_INFO("release buffer ts: {0}",ts.time_since_epoch().count());
        //if (running_buffers_.find(findResult, ts)) {
        const auto& findResult = running_buffers_.find(ts);
        if (findResult != running_buffers_.end()) {

            TimeDomainBufferPtr freeBuffer = findResult->second;
            {
                //std::unique_lock lock(mutex_);
                running_buffers_.erase(findResult);
                free_buffers_.push(freeBuffer);
            }

            //findResult.release();
            return 0;

        }

        throw std::invalid_argument("no domain buffer found for timestamp");

    };

void TimeDomainManager::init(const ComponentGraphPtr &component_graph) {
    source_count_ = 0;
    auto components = component_graph->getPatternsForTimeDomain(time_domain_);
    for(const auto& component : components){
        if(component.second->getComponentType() == component::ComponentType::AsyncSource) {
            source_count_ += component.first->getProducerPorts().size();
        }
    }

  for (size_t index = 0; index < ringbuffer_size_; ++index) {
    free_buffers_.push(std::make_shared<DefaultTimeDomainBuffer>(time_domain_,this, component_graph, generic_factory_objects_));
  }

};
SourceMode TimeDomainManager::getSourceMode() const {
  return source_mode_;
}
void TimeDomainManager::setSourceMode(SourceMode source_mode) {
  source_mode_ = source_mode;
}
size_t TimeDomainManager::GetDomainMeasurementIndex(TimestampType ts) {
    TD_Lock::scoped_lock lock(buffer_mutex_, false);
  //typename RunningBufferType::const_accessor findResult;
    const auto& findResult = running_buffers_.find(ts);
  if (findResult != running_buffers_.end()) {
    std::shared_ptr<DefaultTimeDomainBuffer> foundBuffer = findResult->second;

    return foundBuffer->GetCurrentMeasurementIndex();

  }
  throw std::invalid_argument("no domain buffer found for timestamp");
}

    void TimeDomainManager::registerBufferSource(BufferSource::Ptr buffer_source){
    buffer_sources_.push_back(buffer_source);
}

    void TimeDomainManager::stop(){

        TD_Lock::scoped_lock lock(buffer_mutex_, true);
        // cancel all running buffers
        for(const auto& it : running_buffers_) {
            auto& buffer = it.second;
            buffer->invalidateBuffer();
        }
}

}