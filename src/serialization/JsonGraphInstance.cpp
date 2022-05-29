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

#include <traact/serialization/JsonGraphInstance.h>

namespace ns {

    using nlohmann::json;

    void to_json(json &jobj, const traact::pattern::instance::GraphInstance &obj) {
        jobj["name"] = obj.name;

        json jpatterns;
        for (const auto &pattern_instance : obj.pattern_instances) {
            json jpattern_instance;
            to_json(jpattern_instance, *pattern_instance.second);
            jpatterns.emplace_back(jpattern_instance);
        }

        json jpatterns_td;
        for (const auto &td_configs : obj.timedomain_configs) {
            json jpattern_instance;
            to_json(jpattern_instance, td_configs.second);
            jpatterns_td.emplace_back(jpattern_instance);
        }

        jobj["time_domain_configs"] = jpatterns_td;

        jobj["patterns"] = jpatterns;

    }

    void from_json(const json &jobj, traact::pattern::instance::GraphInstance &obj) {
        obj.name = jobj["name"].get<std::string>();

        auto find_it_td = jobj.find("time_domain_configs");
        if (find_it_td != jobj.end()) {
            const json &jpatterns = *find_it_td;

            for (const auto &jpattern : jpatterns) {
                traact::buffer::TimeDomainManagerConfig config;
                from_json(jpattern, config);
                obj.timedomain_configs.emplace(config.time_domain, config);
            }
        }

        auto find_it = jobj.find("patterns");
        if (find_it != jobj.end()) {
            const json &jpatterns = *find_it;

            for (const auto &jpattern : jpatterns) {
                traact::pattern::instance::PatternInstance::Ptr
                        pattern = std::make_shared<traact::pattern::instance::PatternInstance>();
                from_json(jpattern, *pattern);
                obj.pattern_instances.emplace(std::make_pair(pattern->instance_id, pattern));
            }
        }

        obj.initializeGraphConnections();
    }

} // namespace ns