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

#ifndef TRAACT_INCLUDE_TRAACT_SERIALIZATION_JSONPATTERN_H_
#define TRAACT_INCLUDE_TRAACT_SERIALIZATION_JSONPATTERN_H_

#include <traact/serialization/JsonPort.h>
#include <traact/pattern/Pattern.h>

namespace ns {

using nlohmann::json;

void to_json(json &jobj, const traact::pattern::Pattern &obj) {
  jobj["pattern_name"] = obj.name;
  jobj["concurrency"] = obj.concurrency;

  json::iterator find_it = jobj.find("producer_ports");
  if (!obj.producer_ports.empty()) {
    json jports;
    for (const auto &producer_port : obj.producer_ports) {
      json jport;
      to_json(jport, producer_port);
      jports.emplace_back(jport);
    }
    jobj["producer_ports"] = jports;
  }
  if (!obj.consumer_ports.empty()) {
    json jports;
    for (const auto &producer_port : obj.consumer_ports) {
      json jport;
      to_json(jport, producer_port);
      jports.emplace_back(jport);
    }
    jobj["consumer_ports"] = jports;
  }
}
void from_json(const json &jobj, traact::pattern::Pattern &obj) {
  jobj["pattern_name"].get_to(obj.name);
  jobj["concurrency"].get_to(obj.concurrency);

  auto find_it = jobj.find("producer_ports");
  if (find_it != jobj.end()) {
    json jports = *find_it;

    for (const auto &jproducer_port : jports) {
      traact::pattern::Port port;
      from_json(jproducer_port, port);
      obj.producer_ports.emplace_back(std::move(port));
    }
  }

  find_it = jobj.find("consumer_ports");
  if (find_it != jobj.end()) {
    json jports = *find_it;

    for (const auto &jconsumer_port : jports) {
      traact::pattern::Port port;
      from_json(jconsumer_port, port);
      obj.consumer_ports.emplace_back(std::move(port));
    }
  }

}

} // namespace ns

#endif //TRAACT_INCLUDE_TRAACT_SERIALIZATION_JSONPATTERN_H_
