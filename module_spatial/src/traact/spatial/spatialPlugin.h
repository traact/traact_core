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

#ifndef TRAACT_SPATIAL_MODULE_SRC_TRAACT_SPATIAL_SPATIALPLUGIN_H_
#define TRAACT_SPATIAL_MODULE_SRC_TRAACT_SPATIAL_SPATIALPLUGIN_H_

#include <traact/buffer/GenericFactoryObject.h>
#include <traact/buffer/GenericBufferTypeConversion.h>
#include <traact/datatypes.h>
#include <Eigen/Geometry>

namespace traact::spatial {

namespace type_name {
static const char *Pose6D = "spatial:Pose6D";
}
struct Pose6DHeader {
  /**
   * Only size in byte of element to allocate is used by traact.
   * Everything else
   */
  typedef typename Eigen::Affine3d NativeType;
  const size_t size = sizeof(NativeType);
};

class Pose6DFactoryObject : public buffer::GenericFactoryObject {
 public:
  std::string getTypeName() override {
    return std::move(std::string(type_name::Pose6D));
  }
  void *createObject(void *) override {
    return new Pose6DHeader::NativeType;
  }
  void deleteObject(void *obj) override {
    auto *tmp = static_cast<Pose6DHeader::NativeType *>(obj);
    delete tmp;
  }

};

}

#endif //TRAACT_SPATIAL_MODULE_SRC_TRAACT_SPATIAL_SPATIALPLUGIN_H_
