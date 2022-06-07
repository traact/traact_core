/**
 *   Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com>
 *
 *   License in root folder
**/

#include "ParameterUtils.h"

namespace traact::pattern {

std::set<std::string> CommonParameterEnums::bool_enum = {
    {"true"},
    {"false"}
};

std::map<std::string, bool> CommonParameterEnums::value_to_bool = {
    {"true", true},
    {"false", false}
};

void setBoolValueFromParameter(const nlohmann::json &parameter,
                               std::string parameter_name,
                               bool &paramter_out,
                               bool default_value) {
    if (!parameter.contains(parameter_name)) {
        SPDLOG_WARN("Missing parameter: {0}, using default value: {1}", parameter_name, default_value);
        paramter_out = default_value;
    } else {
        paramter_out = parameter[parameter_name]["value"] == "true";
    }
}
}