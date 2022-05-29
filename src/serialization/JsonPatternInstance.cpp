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

#include <traact/serialization/JsonPatternInstance.h>

namespace ns {

    using nlohmann::json;

    void to_json(json &jobj, const traact::pattern::instance::PatternInstance &obj) {
        jobj["id"] = obj.instance_id;
        jobj["time_domain"] = obj.time_domain;
        to_json(jobj, obj.local_pattern);

        if (!obj.producer_ports.empty()) {
            json &jports = jobj["producer_ports"];
            for (auto &jport : jports) {
                std::string port_name = jport["name"];
                to_json(jport, *obj.getProducerPort(port_name));
            }
        }
        if (!obj.consumer_ports.empty()) {
            json &jports = jobj["consumer_ports"];
            for (auto &jport : jports) {
                std::string port_name = jport["name"];
                to_json(jport, *obj.getConsumerPort(port_name));
            }
        }
    }

    void from_json(const json &jobj, traact::pattern::instance::PatternInstance &obj) {
        jobj["id"].get_to(obj.instance_id);
        from_json(jobj, obj.local_pattern);
        jobj["time_domain"].get_to(obj.time_domain);

        auto find_it = jobj.find("producer_ports");
        if (find_it != jobj.end()) {
            const json &jproducer_ports = *find_it;

            for (const auto &jproducer_port : jproducer_ports) {
                traact::pattern::instance::PortInstance port;
                from_json(jproducer_port, port);
                obj.producer_ports.emplace_back(std::move(port));
            }
        }

        find_it = jobj.find("consumer_ports");
        if (find_it != jobj.end()) {
            const json &jconsumer_ports = *find_it;

            for (const auto &jconsumer_port : jconsumer_ports) {
                traact::pattern::instance::PortInstance port;
                from_json(jconsumer_port, port);
                obj.consumer_ports.emplace_back(std::move(port));
            }
        }

    }

} // namespace ns
