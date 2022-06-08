/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/
#include "JsonTimeDomainManagerConfig.h"
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