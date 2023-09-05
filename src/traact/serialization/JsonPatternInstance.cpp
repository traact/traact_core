/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include <traact/serialization/JsonPatternInstance.h>
#include <traact/serialization/JsonPattern.h>
#include <traact/serialization/JsonPortInstance.h>

namespace ns {

using nlohmann::json;

void to_json(json &jobj, const traact::pattern::instance::PatternInstance &obj) {
    to_json(jobj["local_pattern"], obj.local_pattern);
    jobj["id"] = obj.instance_id;
    jobj["display_name"] = obj.display_name;


    json & json_port_groups = jobj["port_groups_instances"];
    for (const auto& port_group_instances : obj.port_groups) {
        json json_port_group_instances;
        for (const auto& port_group_instance : port_group_instances){
            json json_port_group_instance;
            to_json(json_port_group_instance, *port_group_instance);
            json_port_group_instances.emplace_back(json_port_group_instance);
        }
        json_port_groups.emplace_back(json_port_group_instances);
    }

    jobj["port_group_name_to_index"] = obj.port_group_name_to_index;
    jobj["local_to_global_time_domain"] = obj.local_to_global_time_domain;
}

void from_json(const json &jobj, traact::pattern::instance::PatternInstance &obj) {
    from_json(jobj["local_pattern"], obj.local_pattern);
    jobj.at("id").get_to(obj.instance_id);
    jobj.at("display_name").get_to(obj.display_name);

    auto find_it = jobj.find("port_groups_instances");
    if (find_it != jobj.end()) {
        json json_port_groups = *find_it;
        for (const auto &json_port_group_instances : json_port_groups) {
            std::vector<std::shared_ptr<traact::pattern::instance::PortGroupInstance>> port_group_instances;
            for (int instance_index = 0; instance_index < json_port_group_instances.size(); ++instance_index) {
                auto port_instance = std::make_shared<traact::pattern::instance::PortGroupInstance>();
                from_json(json_port_group_instances[instance_index], *port_instance);
                port_group_instances.emplace_back(port_instance);
            }

            obj.port_groups.emplace_back(std::move(port_group_instances));
        }
    }

    obj.port_group_name_to_index = jobj.at("port_group_name_to_index").get<std::map<std::string, int>>();
    obj.local_to_global_time_domain = jobj.at("local_to_global_time_domain").get<std::vector<int>>();


}

} // namespace ns
