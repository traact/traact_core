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

#ifndef TRAACTMULTI_JSONTIMEDOMAINMANAGERCONFIG_H
#define TRAACTMULTI_JSONTIMEDOMAINMANAGERCONFIG_H

#include <nlohmann/json.hpp>
#include <traact/datatypes.h>
#include <traact/traact_core_export.h>
namespace ns {

    using nlohmann::json;

    void to_json(json &jobj, const traact::buffer::TimeDomainManagerConfig &obj) {
        jobj["time_domain"] = obj.time_domain;
        jobj["ringbuffer_size"] = obj.ringbuffer_size;
        jobj["source_mode"] = static_cast<int>(obj.source_mode);
        jobj["missing_source_event_mode"] = static_cast<int>(obj.missing_source_event_mode);
        jobj["master_source"] = obj.master_source;
        jobj["measurement_delta"] = obj.measurement_delta.count();
        jobj["max_offset"] = obj.max_offset.count();
        jobj["max_delay"] = obj.max_delay.count();

    }

    void from_json(const json &jobj, traact::buffer::TimeDomainManagerConfig &obj) {
        jobj.at("time_domain").get_to(obj.time_domain);
        jobj.at("ringbuffer_size").get_to(obj.ringbuffer_size);
        jobj.at("source_mode").get_to(obj.source_mode);
        jobj.at("missing_source_event_mode").get_to(obj.missing_source_event_mode);
        jobj.at("master_source").get_to(obj.master_source);

        obj.measurement_delta = std::chrono::nanoseconds(jobj.at("measurement_delta").get<int64_t>());
        obj.max_offset = std::chrono::nanoseconds(jobj.at("max_offset").get<int64_t>());
        obj.max_delay = std::chrono::nanoseconds(jobj.at("max_delay").get<int64_t>());
    }

} // namespace ns


#endif //TRAACTMULTI_JSONTIMEDOMAINMANAGERCONFIG_H
