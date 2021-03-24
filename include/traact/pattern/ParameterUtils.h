/*  BSD 3-Clause License
 *
 *  Copyright (c) 2020, FriederPankratz <frieder.pankratz@gmail.com>
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice, this
 *     list of conditions and the following disclaimer.
 *
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *  3. Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 *  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 *  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 *  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#ifndef TRAACTMULTI_TRAACT_CORE_INCLUDE_TRAACT_PATTERN_PARAMETERUTILS_H_
#define TRAACTMULTI_TRAACT_CORE_INCLUDE_TRAACT_PATTERN_PARAMETERUTILS_H_

#include <map>
#include <set>
#include <nlohmann/json.hpp>
#include <traact/traact_core_export.h>
#include <traact/util/Logging.h>

namespace traact::pattern {

void TRAACT_CORE_EXPORT setBoolValueFromParameter(const nlohmann::json& parameter, std::string parameter_name, bool& paramter_out, bool default_value );

template<typename ParaType, typename DefaultValueType>
bool setValueFromParameter(const nlohmann::json& parameter, std::string parameter_name, ParaType& paramter_out, DefaultValueType default_value ) {
  if(!parameter.contains(parameter_name)) {
    SPDLOG_WARN("Missing parameter: {0}, using default value: {1}",parameter_name, default_value);
    paramter_out = default_value;
    return false;
  } else {
    paramter_out = parameter[parameter_name]["value"];
  }
    return true;
}

template<typename ParaType, typename DefaultValueType>
bool setValueFromParameter(const nlohmann::json& parameter, std::string parameter_name, ParaType& paramter_out, DefaultValueType default_value, const std::map<std::string, ParaType>& key_value ) {
  if(!parameter.contains(parameter_name)) {
    SPDLOG_WARN("Missing parameter: {0}, using default value: {1}",parameter_name, default_value);
    paramter_out = key_value.at(default_value);
    return false;
  } else {
    paramter_out = key_value.at(parameter[parameter_name]["value"]);
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
