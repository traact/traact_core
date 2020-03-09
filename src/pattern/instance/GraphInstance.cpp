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

#include <traact/util/Utils.h>
#include "traact/pattern/instance/GraphInstance.h"
traact::pattern::instance::GraphInstance::GraphInstance(const std::string &name) : name(name) {}
traact::pattern::instance::GraphInstance::GraphInstance() : name("Invalid") {

}

traact::pattern::instance::PatternInstance::Ptr traact::pattern::instance::GraphInstance::addPattern(std::string pattern_id,
                                                                                                     traact::pattern::Pattern::Ptr pattern) {
  PatternInstance::Ptr newPattern = std::make_shared<PatternInstance>(pattern_id, *pattern, this);
  pattern_instances[pattern_id] = newPattern;
  return newPattern;
}
traact::pattern::instance::PatternInstance::Ptr traact::pattern::instance::GraphInstance::getPattern(const std::string &pattern_id) const {
  return pattern_instances.at(pattern_id);
}
std::set<traact::pattern::instance::PatternInstance::Ptr> traact::pattern::instance::GraphInstance::getAll() const {
  std::set<PatternInstance::Ptr> result;
  for (const auto &instances : pattern_instances) {
    result.emplace(instances.second);
  }
  return result;
}
traact::pattern::instance::PortInstance::ConstPtr traact::pattern::instance::GraphInstance::getPort(const traact::pattern::instance::ComponentID_PortName &id) const {
  return pattern_instances.at(id.first)->getPort(id.second);
}
void traact::pattern::instance::GraphInstance::initializeGraphConnections() {
  for (auto &pattern_instance : pattern_instances) {
    pattern_instance.second->parent_graph = this;
    for (auto &producer_port : pattern_instance.second->producer_ports) {
      producer_port.pattern_instance = pattern_instance.second.get();
    }
    for (auto &consumer_port : pattern_instance.second->consumer_ports) {
      consumer_port.pattern_instance = pattern_instance.second.get();
    }
  }

}
bool traact::pattern::instance::GraphInstance::connect(std::string source_component,
                                                       std::string producer_port,
                                                       std::string sink_component,
                                                       std::string consumer_port) {


  //todo check
  PortInstance::Ptr source = util::vectorGetForName(pattern_instances[source_component]->producer_ports, producer_port);
  PortInstance::Ptr sink = util::vectorGetForName(pattern_instances[sink_component]->consumer_ports, consumer_port);

  sink->connected_to = source->getID();

  return true;
}
std::set<traact::pattern::instance::PortInstance::ConstPtr> traact::pattern::instance::GraphInstance::connectedToPtr(
    const traact::pattern::instance::ComponentID_PortName &id) const {

  std::set<traact::pattern::instance::PortInstance::ConstPtr> result;

  PortInstance::ConstPtr port = getPort(id);
  if (port->port.porttype == PortType::Consumer) {
    for (const auto &pattern_instance : pattern_instances) {
      for (const auto &producer_port : pattern_instance.second->producer_ports) {
        if (producer_port.getID() == port->connected_to)
          result.emplace(&producer_port);
      }
    }
  } else {
    for (const auto &pattern_instance : pattern_instances) {
      for (const auto &consumer_port : pattern_instance.second->consumer_ports) {
        if (consumer_port.connected_to == id)
          result.emplace(&consumer_port);
      }
    }
  }

  return result;
}

