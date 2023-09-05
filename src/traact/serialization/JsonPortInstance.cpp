/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <traact/serialization/JsonPortInstance.h>
namespace ns {

using nlohmann::json;

void to_json(json &jobj, const traact::pattern::instance::PortInstance &obj) {
    to_json(jobj, obj.port);
    if (obj.port.port_type == traact::pattern::PortType::CONSUMER) {
        jobj["connected_to"] = json(obj.connected_to);
    }

}

void from_json(const json &jobj, traact::pattern::instance::PortInstance &obj) {
    from_json(jobj, obj.port);
    if (obj.port.port_type == traact::pattern::PortType::CONSUMER) {
        auto find_it = jobj.find("connected_to");
        if (find_it != jobj.end()) {
            find_it->get_to(obj.connected_to);
        }
    }

}
void to_json(json &jobj, const traact::pattern::instance::PortGroupInstance &obj) {
    to_json(jobj, obj.port_group);
    jobj["port_group_instance_index"] = obj.port_group_instance_index;

    if(!obj.consumer_ports.empty()) {
        auto &json_consumer_ports = jobj["consumer_ports"];
        for (int port_index = 0; port_index < obj.consumer_ports.size(); ++port_index) {
            to_json(json_consumer_ports.at(port_index), obj.consumer_ports.at(port_index));
        }
    }

    if(!obj.producer_ports.empty()){
        auto &json_producer_ports = jobj["producer_ports"];
        for (int port_index = 0; port_index < obj.producer_ports.size(); ++port_index) {
            to_json(json_producer_ports.at(port_index), obj.producer_ports.at(port_index));
        }
    }

}
void from_json(const json &jobj, traact::pattern::instance::PortGroupInstance &obj) {
    from_json(jobj, obj.port_group);
    jobj.at("port_group_instance_index").get_to(obj.port_group_instance_index);

    auto find_it = jobj.find("consumer_ports");
    if (find_it != jobj.end()) {
        json json_ports = *find_it;
        for (const auto &json_port : json_ports) {
            traact::pattern::instance::PortInstance port_instance;
            from_json(json_port, port_instance);
            obj.consumer_ports.emplace_back(std::move(port_instance));
        }
    }

    find_it = jobj.find("producer_ports");
    if (find_it != jobj.end()) {
        json json_ports = *find_it;
        for (const auto &json_port : json_ports) {
            traact::pattern::instance::PortInstance port_instance;
            from_json(json_port, port_instance);
            obj.producer_ports.emplace_back(std::move(port_instance));
        }
    }
}

} // namespace ns