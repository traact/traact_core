/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

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