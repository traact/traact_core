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

#ifndef TRAACT_INCLUDE_TRAACT_BUFFER_GENERIC_DOUBLETYPE_H_
#define TRAACT_INCLUDE_TRAACT_BUFFER_GENERIC_DOUBLETYPE_H_

#include <cstddef>
#include <traact/buffer/GenericFactoryObject.h>
#include <traact/buffer/GenericBufferTypeConversion.h>
#include <cmath>
#include <traact/datatypes.h>
namespace traact {

namespace type_name {
static const char *TestDouble = "test.double";
}

struct DoubleHeader {
  /**
   * Only size in byte of element to allocate is used by traact.
   * Everything else
   */
  const size_t size = sizeof(NativeType);
  typedef double NativeType;

};

class DoubleFactoryObject : public buffer::GenericFactoryObject {
 public:
  std::string getTypeName() override {
    return std::string(type_name::TestDouble);
  }
  void *createObject(void *) override {
    return new double;
  }
  void deleteObject(void *obj) override {
    auto *tmp = static_cast<double *>(obj);
    delete tmp;
  }

};

namespace buffer {
template<>
double &GenericBufferTypeConversion::asMutable<double, DoubleHeader>(void *obj, void *header) {
  //DoubleHeader *myinfo = static_cast<DoubleHeader *>(info);
  return *static_cast<double *>(obj);
}

template<>
const double &GenericBufferTypeConversion::asImmutable<double, DoubleHeader>(void *obj, void *header) {
  //DoubleHeader *myinfo = static_cast<DoubleHeader *>(info);
  return *static_cast<double *>(obj);
}

template<>
const int &GenericBufferTypeConversion::asImmutable<int, DoubleHeader>(void *obj, void *header) {
  //DoubleHeader *myinfo = static_cast<DoubleHeader *>(info);
  int result = std::round(*static_cast<double *>(obj));
  return std::move(result);
}

}

}

#endif //TRAACT_INCLUDE_TRAACT_BUFFER_GENERIC_DOUBLETYPE_H_
