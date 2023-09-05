/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACTMULTI_TRAACT_CORE_INCLUDE_TRAACT_PATTERN_PARAMETERUTILS_H_
#define TRAACTMULTI_TRAACT_CORE_INCLUDE_TRAACT_PATTERN_PARAMETERUTILS_H_

#include <map>
#include <set>
#include <nlohmann/json.hpp>
#include <traact/traact_core_export.h>
#include <traact/util/Logging.h>
#include <traact/pattern/instance/PatternInstance.h>

namespace traact::pattern {

void TRAACT_CORE_EXPORT setBoolValueFromParameter(const nlohmann::json &parameter,
                                                  std::string parameter_name,
                                                  bool &paramter_out,
                                                  bool default_value);

template<typename ParaType, typename DefaultValueType>
bool setValueFromParameter(const traact::pattern::instance::PatternInstance &pattern_instance,
                           std::string parameter_name,
                           ParaType &parameter_out,
                           DefaultValueType default_value) {
    auto& parameter = pattern_instance.port_groups.at(kDefaultPortGroupIndex)[0]->port_group.parameter;
    if (!parameter.contains(parameter_name)) {
        SPDLOG_WARN("Missing parameter: {0}, using default value: {1}", parameter_name, default_value);
        parameter_out = default_value;
        return false;
    } else {
        parameter_out = parameter[parameter_name]["value"];
    }
    return true;
}

template<typename ParaType, typename DefaultValueType>
bool setValueFromParameter(const traact::pattern::instance::PatternInstance &pattern_instance,
                           std::string parameter_name,
                           ParaType &paramter_out,
                           DefaultValueType default_value,
                           const std::map<std::string, ParaType> &key_value) {
    auto& parameter = pattern_instance.port_groups.at(kDefaultPortGroupIndex)[0]->port_group.parameter;
    if (!parameter.contains(parameter_name)) {
        SPDLOG_WARN("Missing parameter: {0}, using default value: {1}", parameter_name, default_value);
        paramter_out = key_value.at(default_value);
        return false;
    } else {
        std::string key_name = parameter[parameter_name]["value"];
        paramter_out = key_value.at(key_name);
    }
    return true;
}

class TRAACT_CORE_EXPORT CommonParameterEnums {
 public:
    static std::set<std::string> bool_enum;
    static std::map<std::string, bool> value_to_bool;

};

}

#endif //TRAACTMULTI_TRAACT_CORE_INCLUDE_TRAACT_PATTERN_PARAMETERUTILS_H_
