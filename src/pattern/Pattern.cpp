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

#include "traact/pattern/Pattern.h"
#include <traact/util/Utils.h>

traact::pattern::Pattern::Pattern(std::string name, int concurrency)
    : name(std::move(name)), concurrency(concurrency) {}

traact::pattern::Pattern::Pattern() : name("Invalid"), concurrency(0) {

}

traact::pattern::Pattern &traact::pattern::Pattern::addProducerPort(const std::string &name,
                                                                    const std::string &data_meta_type,
                                                                    int port_index) {

  if (util::vectorContainsName(producer_ports, name))
    throw std::invalid_argument("Name of port already in use, Component: " + name + " Port: " + name);

  if (port_index < 0)
    port_index = producer_ports.size();

  if(is_group_port)
      group_ports.back().producer_ports.emplace_back(name, data_meta_type, PortType::Producer, port_index);
  else
      producer_ports.emplace_back(name, data_meta_type, PortType::Producer, port_index);

  return *this;
}
traact::pattern::Pattern &traact::pattern::Pattern::addConsumerPort(const std::string &name,
                                                                    const std::string &data_meta_type, int port_index) {
  if (util::vectorContainsName(consumer_ports, name))
    throw std::invalid_argument("Name of port already in use, Component: " + name + " Port: " + name);

  if (port_index < 0)
    port_index = consumer_ports.size();

    if(is_group_port)
        group_ports.back().consumer_ports.emplace_back(name, data_meta_type, PortType::Consumer, port_index);
    else
        consumer_ports.emplace_back(name, data_meta_type, PortType::Consumer, port_index);
  return *this;
}

traact::pattern::Pattern::~Pattern() {

}

traact::pattern::Pattern &traact::pattern::Pattern::addStringParameter(const std::string &name,
                                                                 const std::string &default_value) {
    if(is_group_port){
        auto& group_parameter = group_ports.back().parameter;
        group_parameter[name]["default"] = default_value;
        group_parameter[name]["value"] = default_value;
    } else {
        parameter[name]["default"] = default_value;
        parameter[name]["value"] = default_value;
    }


  return *this;
}
traact::pattern::Pattern &traact::pattern::Pattern::addParameter(const std::string &name,
                                                                 const std::string &default_value,
                                                                 const std::set<std::string> &enum_values) {
    if(is_group_port){
        auto& group_parameter = group_ports.back().parameter;
        group_parameter[name]["default"] = default_value;
        group_parameter[name]["value"] = default_value;
        group_parameter[name]["enum_values"] = enum_values;
    } else {
        parameter[name]["default"] = default_value;
        parameter[name]["value"] = default_value;
        parameter[name]["enum_values"] = enum_values;
    }

  return *this;
}
traact::pattern::Pattern &traact::pattern::Pattern::addParameter(const std::string &name,
                                                                 const nlohmann::json &json_value) {

    if(is_group_port){
        auto& group_parameter = group_ports.back().parameter;
        group_parameter[name]["json_value"] = json_value;
    } else {
        parameter[name]["json_value"] = json_value;
    }

  return *this;
}

traact::pattern::Pattern &traact::pattern::Pattern::addCoordinateSystem(const std::string &name, bool is_multi) {
    spatial::CoordinateSystem newCoord(name, is_multi);
    if(is_group_port){
        group_ports.back().coordinate_systems_.emplace(std::make_pair(name, std::move(newCoord)));
    } else {
        coordinate_systems_.emplace(std::make_pair(name, std::move(newCoord)));
    }

    return *this;
}
traact::pattern::Pattern &traact::pattern::Pattern::addEdge(const std::string &source,
                                                                                            const std::string &destination,
                                                                                            const std::string &port) {

    //TODO check for valid input
    if(is_group_port){
        group_ports.back().edges_.emplace(std::make_tuple(source, destination, port));
    } else {
        edges_.emplace(std::make_tuple(source, destination, port));
    }

    return *this;
}

traact::pattern::Pattern &traact::pattern::Pattern::beginPortGroup(const std::string &name) {
    is_group_port = true;
    group_ports.emplace_back(PortGroup());
    group_ports.back().name = name;
    return *this;
}

traact::pattern::Pattern &traact::pattern::Pattern::endPortGroup() {
    is_group_port = false;
    return *this;
}
