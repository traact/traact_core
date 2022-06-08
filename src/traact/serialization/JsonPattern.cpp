/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "JsonPattern.h"

namespace ns {

using nlohmann::json;

void to_json(json &jobj, const traact::pattern::Pattern &obj) {
    jobj["pattern_name"] = obj.name;
    jobj["concurrency"] = static_cast<int>(obj.concurrency);
    jobj["parameter"] = obj.parameter;

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
    int concurrency_tmp;
    jobj["concurrency"].get_to(concurrency_tmp);
    obj.concurrency = static_cast<traact::Concurrency>(concurrency_tmp);
    obj.parameter = jobj["parameter"];

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