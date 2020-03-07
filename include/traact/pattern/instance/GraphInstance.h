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

#ifndef TRAACT_INCLUDE_TRAACT_PATTERN_SPATIAL_INSTANTIATEDGRAPH_H_
#define TRAACT_INCLUDE_TRAACT_PATTERN_SPATIAL_INSTANTIATEDGRAPH_H_
#include <map>
#include <memory>
#include <traact/pattern/instance/PatternInstance.h>

namespace traact::pattern::instance {
struct GraphInstance {
 public:
  typedef typename std::shared_ptr<GraphInstance> Ptr;
  GraphInstance();
  GraphInstance(const std::string &name);

  PatternInstance::Ptr addPattern(std::string pattern_id, Pattern::Ptr pattern);

  PatternInstance::Ptr getPattern(const std::string &pattern_id) const;

  std::set<PatternInstance::Ptr> getAll() const;

  bool connect(std::string source_component,
               std::string producer_port,
               std::string sink_component,
               std::string consumer_port);

  traact::pattern::instance::PortInstance::ConstPtr getPort(const ComponentID_PortName &id) const;
  std::set<traact::pattern::instance::PortInstance::ConstPtr> connectedToPtr(const ComponentID_PortName &id) const;

  std::string name;
  std::map<std::string, PatternInstance::Ptr> pattern_instances;

  void initializeGraphConnections();

};
}

#endif //TRAACT_INCLUDE_TRAACT_PATTERN_SPATIAL_INSTANTIATEDGRAPH_H_
