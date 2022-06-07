/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include "JsonPort.h"

namespace ns {

using nlohmann::json;

void to_json(json &jobj, const traact::pattern::Port &obj) {
    jobj["name"] = obj.name;
    jobj["datatype"] = obj.datatype;
    jobj["porttype"] = obj.porttype;
    jobj["port_index"] = obj.port_index;
}

void from_json(const json &jobj, traact::pattern::Port &obj) {
    jobj.at("name").get_to(obj.name);
    jobj.at("datatype").get_to(obj.datatype);
    jobj.at("porttype").get_to(obj.porttype);
    jobj.at("port_index").get_to(obj.port_index);
}

} // namespace ns
