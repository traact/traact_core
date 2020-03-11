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

namespace traact::buffer {

class GenericComponentBuffer;

class GenericTimeDomainBuffer {
 public:
  typedef typename std::shared_ptr<GenericTimeDomainBuffer> Ptr;
  typedef GenericComponentBuffer ComponentBuffer;
  typedef typename std::vector<void *> BufferType;

  GenericTimeDomainBuffer(component::ComponentGraph::Ptr component_graph,
                          const std::set<buffer::GenericFactoryObject::Ptr> &generic_factory_objects);
  virtual ~GenericTimeDomainBuffer();

  GenericComponentBuffer &getComponentBuffer(const std::string &component_name);
  const TimestampType &getTimestamp() const;
  bool isFree() const;
  void resetForTimestamp(TimestampType ts, size_t measurement_index);
  size_t GetCurrentMeasurementIndex() const;
  void decreaseUse();
  int getUseCount() const;

 private:
  component::ComponentGraph::Ptr component_graph_;
  std::map<std::string, buffer::GenericFactoryObject::Ptr> generic_factory_objects_;
  std::atomic<int> current_wait_count_;
  int maximum_wait_count;

  TimestampType current_timestamp_;
  size_t current_measurement_index_;

  std::map<std::string, std::shared_ptr<GenericComponentBuffer> > component_buffers_;
  BufferType buffer_data_;
  std::vector<std::string> types_of_buffer_;

  void addBuffer(const std::string &buffer_type);
};
}

#endif //TRAACT_INCLUDE_TRAACT_BUFFER_GENERIC_GENERICTIMEDOMAINBUFFER_H_
