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

#include <spdlog/spdlog.h>
#include "traact/pattern/instance/PortInstance.h"
#include "traact/pattern/instance/PatternInstance.h"
#include <traact/pattern/instance/GraphInstance.h>
traact::pattern::instance::PortInstance::PortInstance(traact::pattern::Port port,
                                                      traact::pattern::instance::PatternInstance *pattern_instance)
    : port(port), pattern_instance(pattern_instance) {

}

traact::pattern::instance::PortInstance::PortInstance() : port(), pattern_instance(nullptr) {

}

traact::pattern::instance::ComponentID_PortName traact::pattern::instance::PortInstance::getID() const {
  return std::make_pair(pattern_instance->instance_id, getName());
}

const std::string &traact::pattern::instance::PortInstance::getName() const {
  return port.name;
}
const std::string &traact::pattern::instance::PortInstance::getDataType() const {
  return port.datatype;
}
int traact::pattern::instance::PortInstance::getPortIndex() const {
  return port.port_index;
}
std::set<traact::pattern::instance::PortInstance::ConstPtr> traact::pattern::instance::PortInstance::connectedToPtr() const {
  return pattern_instance->parent_graph->connectedToPtr(getID());
}

bool traact::pattern::instance::PortInstance::IsConnected() const {
    return !connectedToPtr().empty();
}
