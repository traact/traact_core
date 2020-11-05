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

#ifndef TRAACT_INCLUDE_TRAACT_BUFFER_GENERIC_GENERICFACTORYOBJECT_H_
#define TRAACT_INCLUDE_TRAACT_BUFFER_GENERIC_GENERICFACTORYOBJECT_H_

#include <string>
#include <memory>
#include <traact/traact_core_export.h>
#include <rttr/type>
namespace traact::buffer {

class TRAACT_CORE_EXPORT GenericFactoryObject : public std::enable_shared_from_this<GenericFactoryObject>{
 public:
  typedef typename std::shared_ptr<GenericFactoryObject> Ptr;
  GenericFactoryObject() = default;
  virtual ~GenericFactoryObject() = default;

  virtual std::string getTypeName() = 0;
  virtual void *createObject() = 0;
  virtual bool initObject(void *header, void* object) = 0;
  virtual void deleteObject(void *obj) = 0;

  template <typename Derived>
  std::shared_ptr<Derived> shared_from_base()
  {
    return std::static_pointer_cast<Derived>(shared_from_this());
  }
  /* Enable RTTR Type Introspection */
 RTTR_ENABLE()
};

/**
 * Default implementation of GenericFactoryObject calling new and delete
 * using the class defines MetaType and NativeType
 * @tparam T
 */
template<class T> class TemplatedDefaultFactoryObject : public GenericFactoryObject{
 public:
  std::string getTypeName() override {
    return std::move(std::string(T::MetaType));
  }
  void *createObject() override {
    return new typename T::NativeType;
  }

  bool initObject(void *header, void* object) override{
    return true;
  }
  void deleteObject(void *obj) override {
    auto *tmp = static_cast<typename T::NativeType *>(obj);
    delete tmp;
  }

  /* Enable RTTR Type Introspection */
 RTTR_ENABLE(GenericFactoryObject)
};

}

#endif //TRAACT_INCLUDE_TRAACT_BUFFER_GENERIC_GENERICFACTORYOBJECT_H_
