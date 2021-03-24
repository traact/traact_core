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

#ifndef TRAACT_INCLUDE_TRAACT_BUFFER_GENERIC_GENERICTIMEDOMAINBUFFER_H_
#define TRAACT_INCLUDE_TRAACT_BUFFER_GENERIC_GENERICTIMEDOMAINBUFFER_H_

#include <atomic>

#include <traact/component/ComponentGraph.h>
#include <traact/buffer/GenericFactoryObject.h>
#include <traact/traact_core_export.h>
#include <tbb/queuing_mutex.h>
#include "GenericBufferTypeConversion.h"
#include "BufferSource.h"
#include "GenericSourceTimeDomainBuffer.h"
#include <tbb/recursive_mutex.h>

namespace traact::buffer {

class TRAACT_CORE_EXPORT GenericComponentBuffer;

class TRAACT_CORE_EXPORT GenericTimeDomainBuffer {
 public:
  typedef GenericComponentBuffer ComponentBuffer;
  typedef typename std::vector<void *> BufferType;
  GenericTimeDomainBuffer(TimeDomainManager *timedomainManager, std::vector<BufferSource*> bufferSources,
                            BufferType bufferData, BufferType bufferHeader,
                            const std::map<pattern::instance::ComponentID_PortName, int>& port_to_bufferIndex, const std::set<pattern::instance::PatternInstance::Ptr>& components);


    virtual ~GenericTimeDomainBuffer();

    GenericComponentBufferConfig* getComponentBufferConfig(const std::string &component_name);
  GenericComponentBuffer &getComponentBuffer(const std::string &component_name);
  const TimestampType &getTimestamp() const;
  bool isFree() const;

  void resetForEvent(size_t event_idx);

  size_t GetCurrentMeasurementIndex() const;
  void decreaseUse();
  void increaseUse();
  int getUseCount() const;
  bool isSourcesSet();
  bool isUsed() const;
  void invalidateBuffer();


    template<typename ReturnType, typename HeaderType>
    const ReturnType& getInput(size_t index) {
        return type_conversion_.asImmutable<ReturnType, HeaderType>(buffer_data_.at(index), 0);

    }

    template<typename HeaderType>
    const HeaderType getInputHeader(size_t index) const {
        return static_cast<HeaderType >(buffer_header_.at(index));
    }

    template<typename ReturnType, typename HeaderType>
    ReturnType &getOutput(size_t index) {
        return type_conversion_.asMutable<ReturnType, HeaderType>(buffer_data_.at(index), 0);
    }


    const void setOutputHeader(size_t index, void* header);

    void SetSourceBuffer(traact::buffer::GenericSourceTimeDomainBuffer *source_buffer);
    bool SetInvalidSourceBuffer(std::size_t source_buffer);

    const std::vector<GenericSourceTimeDomainBuffer*>& GetSourceTimeDomainBuffer() const;

 private:
    //typedef tbb::queuing_mutex LockType;
    typedef tbb::recursive_mutex LockType;
    TimeDomainManager* timedomain_manager_;
    std::vector<BufferSource*> buffer_sources_;
    std::vector<bool> td_buffer_sources_valid_;
    std::vector<bool> td_buffer_sources_send_;
    std::vector<GenericSourceTimeDomainBuffer*> td_buffer_sources_;
    bool is_used_;
    bool is_master_set_;
    LockType source_mutex_;


    std::atomic<int> current_wait_count_;
    int maximum_wait_count_;

    TimestampType current_timestamp_;
    size_t current_measurement_index_;

    std::map<std::string, std::shared_ptr<GenericComponentBuffer> > component_buffers_;
    std::map<std::string, std::shared_ptr<GenericComponentBufferConfig> > component_buffers_config_;
    BufferType buffer_data_;
    BufferType buffer_header_;
    GenericBufferTypeConversion type_conversion_;
};
}

#endif //TRAACT_INCLUDE_TRAACT_BUFFER_GENERIC_GENERICTIMEDOMAINBUFFER_H_
