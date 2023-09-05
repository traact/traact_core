/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <traact/serialization/JsonPort.h>

namespace ns {

using nlohmann::json;

void to_json(json &jobj, const traact::pattern::Port &obj) {
    jobj["name"] = obj.name;
    jobj["datatype"] = obj.datatype;
    jobj["port_type"] = obj.port_type;
    jobj["port_index"] = obj.port_index;
    jobj["time_domain"] = obj.time_domain;

}

void from_json(const json &jobj, traact::pattern::Port &obj) {
    jobj.at("name").get_to(obj.name);
    jobj.at("datatype").get_to(obj.datatype);
    jobj.at("port_type").get_to(obj.port_type);
    jobj.at("port_index").get_to(obj.port_index);
    jobj.at("time_domain").get_to(obj.time_domain);
}

void to_json(json &jobj, const traact::pattern::PortGroup &obj) {
    jobj["name"] = obj.name;
    jobj["group_index"] = obj.group_index;
    jobj["min"] = obj.min;
    jobj["max"] = obj.max;
    jobj["parameter"] = obj.parameter;

    if (!obj.producer_ports.empty()) {
        json &json_ports = jobj["producer_ports"];
        for (const auto &port : obj.producer_ports) {
            json json_port;
            to_json(json_port, port);
            json_ports.emplace_back(json_port);
        }
    }

    if (!obj.consumer_ports.empty()) {
        json &json_ports = jobj["consumer_ports"];
        for (const auto &port : obj.consumer_ports) {
            json json_port;
            to_json(json_port, port);
            json_ports.emplace_back(json_port);
        }
    }

//    if (!obj.coordinate_systems.empty()) {
//        json &json_ports = jobj["coordinate_systems"];
//        for (auto &coordinate_system : obj.coordinate_systems) {
//            auto& json_coord = json_ports[coordinate_system.first];
//            json_coord["is_multi"] = coordinate_system.second.is_multi;
//        }
//    }
// set of edges: source name, destination name, port name
//    if (!obj.edges.empty()) {
//        json &json_ports = jobj["edges"];
//        for (auto &coordinate_system : obj.edges) {
//
//            json_ports.emplace()
//            auto& json_coord = json_ports[coordinate_system.first];
//            json_coord["is_multi"] = coordinate_system.second.is_multi;
//        }
//    }

}
void from_json(const json &jobj, traact::pattern::PortGroup &obj) {
    jobj.at("name").get_to( obj.name);
    jobj.at("group_index").get_to( obj.group_index);
    jobj.at("min").get_to( obj.min);
    jobj.at("max").get_to( obj.max);
    jobj.at("parameter").get_to( obj.parameter);


    auto find_it = jobj.find("producer_ports");
    if (find_it != jobj.end()) {
        json json_ports = *find_it;
        for (const auto &json_producer_port : json_ports) {
            traact::pattern::Port port;
            from_json(json_producer_port, port);
            obj.producer_ports.emplace_back(std::move(port));
        }
    }

    find_it = jobj.find("consumer_ports");
    if (find_it != jobj.end()) {
        json json_ports = *find_it;
        for (const auto &json_consumer_port : json_ports) {
            traact::pattern::Port port;
            from_json(json_consumer_port, port);
            obj.consumer_ports.emplace_back(std::move(port));
        }
    }

}

} // namespace ns
