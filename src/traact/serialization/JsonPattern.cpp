/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <traact/serialization/JsonPattern.h>

namespace ns {

using nlohmann::json;

void to_json(json &jobj, const traact::pattern::Pattern &obj) {
    jobj["pattern_name"] = obj.name;
    jobj["display_name"] = obj.display_name;
    jobj["description"] = obj.description;
    jobj["tags"] = obj.tags;

    auto& json_concurrency = jobj["time_domains"];

    for (size_t i = 0; i < obj.time_domain_component_type.size(); ++i) {
        json type_td;
        type_td["component_type"] = obj.time_domain_component_type[i];
        type_td["concurrency"] = obj.concurrency[i];
        json_concurrency.emplace_back(type_td);
    }

    auto& json_port_groups = jobj["port_groups"];
    for (const auto & port_group : obj.port_groups) {
        json json_port_group;
        to_json(json_port_group, port_group);
        json_port_groups.emplace_back(json_port_group);
    }
}
void from_json(const json &jobj, traact::pattern::Pattern &obj) {
    jobj["pattern_name"].get_to(obj.name);
    jobj["display_name"].get_to(obj.display_name);
    jobj["description"].get_to(obj.description);
    jobj["tags"].get_to(obj.tags);

    auto find_it = jobj.find("time_domains");
    if (find_it != jobj.end()) {
        json json_time_domains = *find_it;
        for (const auto &json_type_td : json_time_domains) {
            obj.time_domain_component_type.emplace_back(json_type_td["component_type"].get<traact::component::ComponentType>());
            obj.concurrency.emplace_back(json_type_td["concurrency"].get<traact::Concurrency>());
        }
    }

    find_it = jobj.find("port_groups");
    if (find_it != jobj.end()) {
        obj.port_groups.clear();
        json json_port_groups = *find_it;
        for (const auto &json_port_group : json_port_groups) {
            traact::pattern::PortGroup port_group;
            from_json(json_port_group, port_group);
            obj.port_groups.emplace_back(std::move(port_group));
        }
    }

}

} // namespace ns