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

#ifndef TRAACT_INCLUDE_TRAACT_TRAACT_H_
#define TRAACT_INCLUDE_TRAACT_TRAACT_H_

#include <traact/datatypes.h>
#include <traact/component/Component.h>
#include <traact/component/ModuleComponent.h>
#include <traact/pattern/Pattern.h>
#include <traact/pattern/ParameterUtils.h>
#include <traact/pattern/spatial/SpatialPattern.h>
#include <traact/pattern/instance/GraphInstance.h>
#include <traact/dataflow/Network.h>
#include <traact/buffer/TimeDomainManager.h>

#include <traact/buffer/GenericComponentBuffer.h>
#include <traact/buffer/GenericTimeDomainBuffer.h>
#include <traact/util/TraactCoreUtils.h>
#include <traact/traact_core_export.h>

namespace traact {

typedef typename buffer::TimeDomainManager DefaultTimeDomainManager;

typedef typename DefaultTimeDomainManager::ComponentGraph DefaultComponentGraph;
typedef typename DefaultTimeDomainManager::ComponentGraphPtr DefaultComponentGraphPtr;
typedef typename DefaultTimeDomainManager::DefaultComponentBuffer DefaultComponentBuffer;

typedef typename component::Component DefaultComponent;
typedef typename DefaultComponent::Ptr DefaultComponentPtr;

typedef typename pattern::Pattern DefaultPattern;
typedef typename DefaultPattern::Ptr DefaultPatternPtr;
typedef typename pattern::instance::GraphInstance DefaultInstanceGraph;
typedef typename pattern::instance::GraphInstance::Ptr DefaultInstanceGraphPtr;
typedef typename pattern::instance::PatternInstance DefaultPatternInstance;
typedef typename pattern::instance::PatternInstance::Ptr DefaultPatternInstancePtr;

typedef typename dataflow::Network DefaultTraactNetwork;
}

#endif //TRAACT_INCLUDE_TRAACT_TRAACT_H_
