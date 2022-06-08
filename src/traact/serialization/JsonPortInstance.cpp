/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#include "JsonPortInstance.h"
namespace ns {

using nlohmann::json;

void to_json(json &jobj, const traact::pattern::instance::PortInstance &obj) {
    to_json(jobj, obj.port);
    if (obj.port.porttype == traact::pattern::PortType::Consumer)
        jobj["connected_to"] = json(obj.connected_to);

}

void from_json(const json &jobj, traact::pattern::instance::PortInstance &obj) {
    from_json(jobj, obj.port);
    if (obj.port.porttype == traact::pattern::PortType::Consumer) {
        auto find_it = jobj.find("connected_to");
        if (find_it != jobj.end())
            find_it->get_to(obj.connected_to);
    }

}

} // namespace ns