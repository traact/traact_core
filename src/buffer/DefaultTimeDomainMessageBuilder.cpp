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

#include "DefaultTimeDomainMessageBuilder.h"

namespace traact::buffer{

    GenericSourceTimeDomainBuffer *
    DefaultTimeDomainMessageBuilder::RequestSourceBuffer(const traact::TimestampType ts,
                                                                         const std::string &component_name) {

        SPDLOG_TRACE("request buffer {0} at ts {1} : find mea idx", component_name, ts.time_since_epoch().count());

        if(ts == TimestampType::min()){
            SPDLOG_ERROR("invalid timestamp 0 for component {0}", component_name);
            return nullptr;
        }
        std::size_t source_index = name_to_buffer_source_.at(component_name);
        if(ts <= latest_ts_[source_index]){
            SPDLOG_ERROR("invalid timestamp ts {1} <= latest {2} for component {0}", component_name,ts.time_since_epoch().count(), latest_ts_[source_index].time_since_epoch().count());
            return nullptr;
        }
        latest_ts_[source_index] = ts;

        // is it the Init call?
        if(ts.time_since_epoch().count() == 1){
            std::size_t source_index = name_to_buffer_source_.at(component_name);
            // wait for free source buffer
            free_source_buffer_lock_[source_index]->wait();
            GenericSourceTimeDomainBuffer* buffer = nullptr;
            if(free_source_buffer_[source_index].try_pop(buffer)) {
                buffer->Init(ts, 0);
                return buffer;
            }
            spdlog::error("could not pop free source buffer for component {0} at ts {1} even though one should be available", component_name, ts.time_since_epoch().count());
            throw std::invalid_argument("could not pop free source buffer for component");
        }

        if(!index_calc_init_){
            if(component_name == config_.master_source){
                TD_Lock::scoped_lock lock(buffer_mutex_);
                // fist real measurement is at idx 1
                first_master_ts_ = ts;
                current_mea_idx_ = 1;
                next_mea_idx_ = 1;
                for(int init_index = 0; init_index < all_td_buffer_.size(); ++init_index){
                    std::size_t nextMeaIdx = next_mea_idx_++;
                    TimestampType nextTs = GetMeaIdxTs(nextMeaIdx);
                    //std::size_t ring_index = GetTDBufferIdx(nextMeaIdx);
                    auto buffer = all_td_buffer_[init_index];

                    if(buffer->isUsed()){
                        spdlog::error("td buffer was still in use");
                    }

                    buffer->resetForTimestamp(nextTs, nextMeaIdx);
                    td_ringbuffer_.emplace(nextMeaIdx, buffer);
                }
                SPDLOG_DEBUG("initialize time domain {0} with first timestamp at {1}", config_.time_domain, first_master_ts_.time_since_epoch().count());
                index_calc_init_ = true;
                wait_for_index_index_lock.SetInit(true);
            } else {
                wait_for_index_index_lock.Wait();
            }
        }

        std::size_t meaIdx;
        if(GetMeaIdx(ts, meaIdx)){
            SPDLOG_TRACE("request buffer {0} at ts {1} : meaIdx {2}", component_name, ts.time_since_epoch().count(), meaIdx);

        } else {
            SPDLOG_ERROR("request buffer {0} at ts {1} : timestamp could not be transformed into valid measurement index, timestamps unstable?", component_name, ts.time_since_epoch().count());
            return nullptr;
        }



        // wait for free source buffer buffer
        // TODO switch sourceMode here?
        free_source_buffer_lock_[source_index]->wait();
        GenericSourceTimeDomainBuffer* buffer = nullptr;
        if(free_source_buffer_[source_index].try_pop(buffer)) {
            buffer->Init(ts, meaIdx);
            source_mea_idx_[source_index] = meaIdx;
            return buffer;
        }
        spdlog::error("could not pop free source buffer for component {0} at ts {1} even though one should be available", component_name, ts.time_since_epoch().count());
        throw std::invalid_argument("could not pop free source buffer for component");
    }

    GenericSourceTimeDomainBuffer *
    DefaultTimeDomainMessageBuilder::RequestNextSourceBuffer(const std::string &component_name) {
        auto source_index = name_to_buffer_source_.at(component_name);
        TimestampType nextTs = GetMeaIdxTs(source_mea_idx_[source_index]+1);
        return RequestSourceBuffer(nextTs, component_name);
    }

    bool DefaultTimeDomainMessageBuilder::CommitSourceBuffer(GenericSourceTimeDomainBuffer *buffer, bool valid) {

        TD_Lock::scoped_lock lock(buffer_mutex_);
        SPDLOG_TRACE("commit source buffer {0} at meaIdx {1}, ts {2}",buffer->GetSourceTDBufferIndex(), buffer->GetMeaIdx(), buffer->GetTs().time_since_epoch().count());

        if(buffer->GetMessageType() == MessageType::Parameter){
            configure_message_td_buffer_->SetSourceBuffer(buffer);
            return true;
        }

        std::size_t mea_idx = buffer->GetMeaIdx();
        std::size_t source_idx = buffer->GetSourceTDBufferIndex();



        {
            if(mea_idx < current_mea_idx_){
                spdlog::error("received source buffer for a meaIdx that already finished, current {0}, received {1}", current_mea_idx_, mea_idx);
                free_source_buffer_[source_idx].push(buffer);
                free_source_buffer_lock_[source_idx]->notify();
                return false;
            }

            if(mea_idx > GetMaxMeaIdx()){
                SPDLOG_TRACE("source buffer {0} at meaIdx {1}, in future meaIdx, push into waiting queue",buffer->GetSourceTDBufferIndex(), buffer->GetMeaIdx(), buffer->GetTs().time_since_epoch().count());
                //TD_Lock::scoped_lock lock(buffer_mutex_);
                unassigned_source_buffer_[source_idx].push_back(buffer);
                //auto oldest_in_list = unassigned_source_buffer_[source_idx].front();
                InvalidateOlderTD(mea_idx, source_idx);
                return true;
            }

            auto result_buffer = td_ringbuffer_.find(mea_idx);
            if(result_buffer == td_ringbuffer_.end()){
                throw std::runtime_error("could not find time domain buffer");
            }
            InvalidateOlderTD(mea_idx, source_idx);
            if(valid){
                result_buffer->second->SetSourceBuffer(buffer);
            } else {
                result_buffer->second->SetInvalidSourceBuffer(source_idx);
                free_source_buffer_[source_idx].push(buffer);
                free_source_buffer_lock_[source_idx]->notify();
            }




        }


        return true;
    }

    void DefaultTimeDomainMessageBuilder::ReleaseTimeDomainBuffer(
            GenericTimeDomainBuffer *td_buffer) {
        SPDLOG_TRACE("release td buffer: ts {0} MeaIndex {1}, pointer {2}", td_buffer->getTimestamp().time_since_epoch().count(), td_buffer->GetCurrentMeasurementIndex(), (uint64_t)td_buffer);

        {
            TD_Lock::scoped_lock lock(buffer_mutex_);

            std::size_t finished_mea_idx = td_buffer->GetCurrentMeasurementIndex();

            if(finished_mea_idx > 0) {
                auto it = td_ringbuffer_.find(finished_mea_idx);
                if(it == td_ringbuffer_.end()){
                    spdlog::error("time domain buffer already released, error in dataflow");
                }
            }


            const auto& source_td_buffer = td_buffer->GetSourceTimeDomainBuffer();
            for(const auto& source_buffer : source_td_buffer){
                if(source_buffer != nullptr){
                    std::size_t buffer_index = source_buffer->GetSourceTDBufferIndex();
                    free_source_buffer_[buffer_index].push(source_buffer);
                    free_source_buffer_lock_[buffer_index]->notify();
                }
            }

            if(td_buffer == configure_message_td_buffer_){
                SPDLOG_INFO("finished with configure message run");
                return;
            }




            std::size_t nextMeaIdx = next_mea_idx_++;
            TimestampType nextTs = GetMeaIdxTs(nextMeaIdx);
            td_ringbuffer_.erase(finished_mea_idx);
            td_buffer->resetForTimestamp(nextTs, nextMeaIdx);
            td_ringbuffer_.emplace(nextMeaIdx, td_buffer);

            //current_mea_idx_++;
            current_mea_idx_ = GetMinRunningIndex();

            // go through all waiting messages
            for(auto& buffer_list : unassigned_source_buffer_){
                if(buffer_list.empty())
                    continue;

                auto buffer = buffer_list.front();
                if(nextMeaIdx == buffer->GetMeaIdx()){
                    buffer_list.pop_front();
                    td_buffer->SetSourceBuffer(buffer);
                } else {
                    td_buffer->SetInvalidSourceBuffer(buffer->GetSourceTDBufferIndex());
                }


            }
        }
    }

    void DefaultTimeDomainMessageBuilder::Stop() {
        TD_Lock::scoped_lock lock(buffer_mutex_);
        if(!initialized_)
            return;
        // cancel all running buffers
        for(auto& buffer : td_ringbuffer_){
            buffer.second->invalidateBuffer();
        }


        initialized_ = false;
    }

    bool DefaultTimeDomainMessageBuilder::InternalCommitBuffer(GenericSourceTimeDomainBuffer *buffer) {
        SPDLOG_TRACE("commit source buffer {0} at meaIdx {1}, ts {2}",buffer->GetSourceTDBufferIndex(), buffer->GetMeaIdx(), buffer->GetTs().time_since_epoch().count());

        if(buffer->GetMessageType() == MessageType::Parameter){
            configure_message_td_buffer_->SetSourceBuffer(buffer);
            return true;
        }

        std::size_t mea_idx = buffer->GetMeaIdx();
        std::size_t source_idx = buffer->GetSourceTDBufferIndex();



        {
            if(mea_idx < current_mea_idx_){
                spdlog::error("received source buffer for a meaIdx that already finished, current {0}, received {1}", current_mea_idx_, mea_idx);
                free_source_buffer_[source_idx].push(buffer);
                free_source_buffer_lock_[source_idx]->notify();
                return false;
            }

            if(mea_idx > GetMaxMeaIdx()){
                SPDLOG_TRACE("source buffer {0} at meaIdx {1}, in future meaIdx, push into waiting queue",buffer->GetSourceTDBufferIndex(), buffer->GetMeaIdx(), buffer->GetTs().time_since_epoch().count());
                //TD_Lock::scoped_lock lock(buffer_mutex_);
                unassigned_source_buffer_[source_idx].push_back(buffer);
                auto oldest_in_list = unassigned_source_buffer_[source_idx].front();
                InvalidateOlderTD(oldest_in_list->GetMeaIdx(), source_idx);
                return true;
            }

            auto result_buffer = td_ringbuffer_.find(mea_idx);
            if(result_buffer == td_ringbuffer_.end()){
                throw std::runtime_error("could not find time domain buffer");
            }
            InvalidateOlderTD(mea_idx, source_idx);
            result_buffer->second->SetSourceBuffer(buffer);


//            auto mea_idx_dist = mea_idx - current_mea_idx;
//            if(mea_idx_dist >= config_.ringbuffer_size-1){
//                SPDLOG_WARN("all time domain buffers used, try cancel source idx {0} ", source_idx);
//                InvalidateOlderTD(mea_idx, source_idx);
//            }

        }


        return true;
    }

    void DefaultTimeDomainMessageBuilder::InvalidateOlderTD(std::size_t mea_idx, std::size_t source_idx) {
        //TD_Lock::scoped_lock lock(buffer_mutex_);
        // the data is far in the future, cancel all older td
        if(mea_idx > GetMaxMeaIdx()) {
            SPDLOG_WARN("cancel all td buffer for source, meaIdx {0} in far future, current {1}", mea_idx, GetMaxMeaIdx());
            for(auto td_buffer : td_ringbuffer_){
                if(td_buffer.second->SetInvalidSourceBuffer(source_idx)) {
                    SPDLOG_WARN("cancel source idx {0} at meaIdx {1}, event missing meaIdx {2}", source_idx, mea_idx, td_buffer.second->GetCurrentMeasurementIndex());
                }
            }

        } // cancel only older
        else {
            //for(std::size_t idx = current_mea_idx; idx < mea_idx;++idx){
            for(auto td_buffer : td_ringbuffer_){
                //auto older_td_ring_idx = GetTDBufferIdx(idx);
                if(td_buffer.second->GetCurrentMeasurementIndex() < mea_idx)
                    if(td_buffer.second->SetInvalidSourceBuffer(source_idx)) {
                        SPDLOG_WARN("cancel source idx {0} at meaIdx {1}, event missing meaIdx {2}", source_idx, mea_idx, td_buffer.second->GetCurrentMeasurementIndex());
                    }
                else
                    break; // map is ordered
            }

        }
    }

    DefaultTimeDomainMessageBuilder::DefaultTimeDomainMessageBuilder(const TimeDomainManagerConfig &config,
                                                                     const std::map<std::string, size_t> &nameToBufferSource,
                                                                     const std::vector<GenericTimeDomainBuffer *> &allTdBuffer,
                                                                     GenericTimeDomainBuffer *configureMessageTdBuffer,
                                                                     const std::vector<std::vector<GenericSourceTimeDomainBuffer *>> &allSourceBuffer)
            : TimeDomainMessageBuilder(config, nameToBufferSource, allTdBuffer, configureMessageTdBuffer,
                                       allSourceBuffer) {
        free_source_buffer_.resize(allSourceBuffer.size());
        // Init semaphores
        //td_ringbuffer_lock_ = std::make_shared<Semaphore>(config_.ringbuffer_size, config_.ringbuffer_size);
        std::size_t source_count = all_source_buffer_.size();
        free_source_buffer_lock_.resize(source_count);
        latest_ts_.resize(source_count, TimestampType::min());
        source_mea_idx_.resize(source_count, 0);
        free_source_buffer_.resize(source_count);
        unassigned_source_buffer_.resize(source_count);
        for(int i=0;i< free_source_buffer_lock_.size();++i){
            free_source_buffer_lock_[i] = std::make_shared<Semaphore>(config_.ringbuffer_size, config_.ringbuffer_size);
            for(auto tmp : allSourceBuffer[i]){
                free_source_buffer_[i].push(tmp);
            }
        }
        initialized_ = true;
    }

    std::size_t DefaultTimeDomainMessageBuilder::GetMinRunningIndex() {
        std::size_t result = std::numeric_limits<std::size_t>::max();
        for(const auto& tmp : td_ringbuffer_){
            result = std::min(result, tmp.first);
        }
        return result;
    }

    void DefaultTimeDomainMessageBuilder::Configure() {

    }

    void DefaultTimeDomainMessageBuilder::Start() {

    }

    void DefaultTimeDomainMessageBuilder::Teardown() {

    }


}

