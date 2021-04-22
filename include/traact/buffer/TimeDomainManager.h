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
#include <traact/buffer/TimeDomainBuffer.h>
#include <traact/buffer/ComponentBuffer.h>
#include <traact/buffer/BufferSource.h>
#include <traact/util/Logging.h>
#include <traact/traact_core_export.h>
#include <list>
#include "SourceTimeDomainBuffer.h"

namespace traact::component {
class ComponentGraph;
}

namespace traact::buffer {



class TRAACT_CORE_EXPORT TimeDomainManager {
 public:
  typedef typename std::shared_ptr<TimeDomainManager> Ptr;
  typedef TimeDomainBuffer DefaultTimeDomainBuffer;
  typedef ComponentBuffer DefaultComponentBuffer;

  typedef typename component::ComponentGraph ComponentGraph;
  typedef typename component::ComponentGraph::Ptr ComponentGraphPtr;


  TimeDomainManager(TimeDomainManagerConfig config,
                    std::set<buffer::BufferFactory::Ptr> factory_objects);

    virtual ~TimeDomainManager();

    void RegisterBufferSource(BufferSource::Ptr buffer_source);

  //SourceTimeDomainBuffer* RequestBuffer(const TimestampType ts, const std::string &component_name);
  //bool CommitSourceBuffer(SourceTimeDomainBuffer *buffer, bool valid);
  //void ReleaseTimeDomainBuffer(TimeDomainBuffer* td_buffer);

    virtual void Init(const ComponentGraphPtr &component_graph);


    virtual SourceTimeDomainBuffer *RequestSourceBuffer(const TimestampType ts, const std::string &component_name) = 0;

    virtual bool CommitSourceBuffer(SourceTimeDomainBuffer *buffer, bool valid) =0 ;

    virtual void ReleaseTimeDomainBuffer(TimeDomainBuffer *td_buffer) = 0;




    virtual void Configure() = 0;
    virtual void Start() = 0;
    virtual void Stop() = 0;
    virtual void Teardown() = 0;

  SourceMode GetSourceMode() const;


    void SetOutputHeader(std::size_t buffer_index, void *header);

protected:


    TimeDomainManagerConfig config_;

    std::map<std::string, buffer::BufferFactory::Ptr> factory_objects_;

    ComponentGraphPtr component_graph_;

    std::vector<ComponentGraph::PatternComponentPair> source_components_;
    std::vector<BufferSource*> buffer_sources_;
    std::map<std::string, size_t> name_to_buffer_source_;

    std::vector<TimeDomainBuffer*> td_ringbuffer_list_;

    std::vector<std::vector<SourceTimeDomainBuffer*> > all_source_buffer_;

    std::vector<std::string> buffer_datatype_;
    std::vector<std::vector<void*> > buffer_data_;
    std::vector<std::vector<void*> > buffer_header_;
    std::map<pattern::instance::ComponentID_PortName, int> port_to_bufferIndex_;




};

}
#endif //TRAACTTEST_INCLUDE_TRAACT_TIMEPOINTBUFFER_TIMEDOMAINMANAGER_H_
