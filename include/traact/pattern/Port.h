#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nodiscard" // class should be used in c++ 11 inferfaces
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

#ifndef TRAACT_INCLUDE_TRAACT_PATTERN_DATAFLOWPORT_H_
#define TRAACT_INCLUDE_TRAACT_PATTERN_DATAFLOWPORT_H_

#include <string>
#include <set>
#include <memory>
#include <traact/traact_core_export.h>
#include <traact/pattern/CoordinateSystem.h>
#include <vector>
#include <nlohmann/json.hpp>

namespace traact::pattern {

    enum class TRAACT_CORE_EXPORT PortType {
        NONE = 0,
        Producer,
        Consumer
    };

//typedef typename std::pair<std::string, std::string> ComponentName_PortName;


    struct TRAACT_CORE_EXPORT Port {
        Port();

        Port(std::string name, std::string datatype, PortType port_type, int port_index);

        const std::string &getName() const;

        std::string name;
        std::string datatype;
        PortType porttype;
        int port_index;

    };

    struct TRAACT_CORE_EXPORT PortGroup {
        std::vector<Port> producer_ports;
        std::vector<Port> consumer_ports;

        std::map<std::string, spatial::CoordinateSystem> coordinate_systems_;
        // set of edges: source name, destination name, port name
        std::set<std::tuple<std::string, std::string, std::string> > edges_;
        nlohmann::json parameter;
        std::string name;
    };
}

#endif //TRAACT_INCLUDE_TRAACT_PATTERN_DATAFLOWPORT_H_

#pragma clang diagnostic pop