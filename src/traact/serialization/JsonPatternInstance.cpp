/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "JsonPatternInstance.h"

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
