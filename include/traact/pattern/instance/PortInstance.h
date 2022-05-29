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

#ifndef TRAACT_INCLUDE_TRAACT_PATTERN_INSTANTIATEDPORT_H_
#define TRAACT_INCLUDE_TRAACT_PATTERN_INSTANTIATEDPORT_H_

#include <traact/pattern/Port.h>
#include <traact/traact_core_export.h>
#include <optional>
namespace traact::pattern::instance {

class TRAACT_CORE_EXPORT PatternInstance;

//
typedef typename std::pair<std::string, std::string> ComponentID_PortName;

struct TRAACT_CORE_EXPORT PortInstance {
  typedef PortInstance *Ptr;
  typedef const PortInstance *ConstPtr;

  PortInstance();
  PortInstance(Port port, PatternInstance *pattern_instance);

  const std::string &getName() const;

  const std::string &getDataType() const;

  int getPortIndex() const;

  std::set<traact::pattern::instance::PortInstance::ConstPtr> connectedToPtr() const;

  ComponentID_PortName getID() const;

  bool IsConnected() const;

  PortType GetPortType() const;

  Port port;
  bool is_active;
  ComponentID_PortName connected_to;
  PatternInstance *pattern_instance;

};

}

#endif //TRAACT_INCLUDE_TRAACT_PATTERN_INSTANTIATEDPORT_H_
