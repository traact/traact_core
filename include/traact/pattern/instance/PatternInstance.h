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

#ifndef TRAACT_INCLUDE_TRAACT_PATTERN_INSTANTIATEDPATTERN_H_
#define TRAACT_INCLUDE_TRAACT_PATTERN_INSTANTIATEDPATTERN_H_
#include <traact/pattern/instance/PortInstance.h>
#include <traact/pattern/Pattern.h>
#include <traact/datatypes.h>
#include <traact/traact_core_export.h>

namespace traact::pattern::instance {

class TRAACT_CORE_EXPORT GraphInstance;

class TRAACT_CORE_EXPORT PatternInstance {
 public:
    PatternInstance(std::string id, bool ismaster, TimeDurationType maxoffset, Pattern pattern_pointer,
                    GraphInstance *graph);

    typedef typename std::shared_ptr<PatternInstance> Ptr;
  PatternInstance();
  virtual ~PatternInstance();

  std::string getPatternName() const;
  size_t getConcurrency() const;


  PortInstance::ConstPtr getProducerPort(const std::string &name) const;
  PortInstance::ConstPtr getConsumerPort(const std::string &name) const;
  PortInstance::ConstPtr getPort(const std::string &name) const;
  std::set<PortInstance::ConstPtr> getProducerPorts() const;
  std::set<PortInstance::ConstPtr> getConsumerPorts() const;

  std::string instance_id;
  GraphInstance *parent_graph;
  Pattern pattern_pointer;
  std::vector<PortInstance> producer_ports;
  std::vector<PortInstance> consumer_ports;
  int time_domain{0};
  bool is_master;
  TimeDurationType max_offset;



};
}

#endif //TRAACT_INCLUDE_TRAACT_PATTERN_INSTANTIATEDPATTERN_H_
