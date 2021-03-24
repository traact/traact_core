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

#include "traact/pattern/instance/PatternInstance.h"
#include <traact/util/Utils.h>


traact::pattern::instance::PatternInstance::PatternInstance(std::string id,
                                                            traact::pattern::Pattern pattern_pointer, traact::pattern::instance::GraphInstance *graph)
    : pattern_pointer(std::move(pattern_pointer)), instance_id(std::move(id)), parent_graph(graph)
    {
  for (const auto &consumer_port : pattern_pointer.consumer_ports) {
    PortInstance newPort(consumer_port, this);
    consumer_ports.emplace_back(std::move(newPort));
  }
  for (const auto &producer_port : pattern_pointer.producer_ports) {
    PortInstance newPort(producer_port, this);
    producer_ports.emplace_back(std::move(newPort));
  }

}

traact::pattern::instance::PatternInstance::PatternInstance()
    : pattern_pointer(), instance_id(""), parent_graph(nullptr) {

}

traact::pattern::instance::PatternInstance::~PatternInstance() {

}

traact::pattern::instance::PortInstance::ConstPtr traact::pattern::instance::PatternInstance::getProducerPort(const std::string &name) const {
  return util::vectorGetForName(producer_ports, name);
}
traact::pattern::instance::PortInstance::ConstPtr traact::pattern::instance::PatternInstance::getConsumerPort(const std::string &name) const {
  return util::vectorGetForName(consumer_ports, name);
}
std::set<traact::pattern::instance::PortInstance::ConstPtr> traact::pattern::instance::PatternInstance::getProducerPorts() const {
  std::set<traact::pattern::instance::PortInstance::ConstPtr> result;
  for (const auto &port : producer_ports) {
    result.emplace(&port);
  }
  return result;
}
std::set<traact::pattern::instance::PortInstance::ConstPtr> traact::pattern::instance::PatternInstance::getConsumerPorts() const {
  std::set<traact::pattern::instance::PortInstance::ConstPtr> result;
  for (const auto &port : consumer_ports) {
    result.emplace(&port);
  }
  return result;
}
size_t traact::pattern::instance::PatternInstance::getConcurrency() const {
  return pattern_pointer.concurrency;
}
traact::pattern::instance::PortInstance::ConstPtr traact::pattern::instance::PatternInstance::getPort(const std::string &name) const {

  if (util::vectorContainsName(producer_ports, name))
    return util::vectorGetForName(producer_ports, name);
  if (util::vectorContainsName(consumer_ports, name))
    return util::vectorGetForName(consumer_ports, name);
  return nullptr;
}
std::string traact::pattern::instance::PatternInstance::getPatternName() const {
  return pattern_pointer.name;
}

