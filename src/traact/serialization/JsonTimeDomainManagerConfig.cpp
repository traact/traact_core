/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/
#include "JsonTimeDomainManagerConfig.h"
namespace ns {

using nlohmann::json;

void to_json(json &jobj, const traact::buffer::TimeDomainManagerConfig &obj) {
    jobj["time_domain"] = obj.time_domain;
    jobj["ringbuffer_size"] = obj.ringbuffer_size;
    jobj["max_offset"] = obj.max_offset.count();
    jobj["max_delay"] = obj.max_delay.count();
    jobj["sensor_frequency"] = obj.sensor_frequency;
    jobj["source_mode"] = static_cast<int>(obj.source_mode);
    jobj["missing_source_event_mode"] = static_cast<int>(obj.missing_source_event_mode);
    jobj["cpu_count"] = obj.cpu_count;
}

void from_json(const json &jobj, traact::buffer::TimeDomainManagerConfig &obj) {
    jobj.at("time_domain").get_to(obj.time_domain);
    jobj.at("ringbuffer_size").get_to(obj.ringbuffer_size);
    obj.max_offset = std::chrono::nanoseconds(jobj.at("max_offset").get<int64_t>());
    obj.max_delay = std::chrono::nanoseconds(jobj.at("max_delay").get<int64_t>());
    obj.sensor_frequency = jobj.at("sensor_frequency").get<double>();
    jobj.at("source_mode").get_to(obj.source_mode);
    jobj.at("missing_source_event_mode").get_to(obj.missing_source_event_mode);
    jobj.at("cpu_count").get_to(obj.cpu_count);



}

} // namespace ns