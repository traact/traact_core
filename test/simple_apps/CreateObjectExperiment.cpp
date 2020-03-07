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

#include <algorithm>
#include <cmath>
#include <iostream>

struct TraactTypeConversion {
  template<typename ReturnType, typename ObjectInfo>
  static ReturnType &asMutable(void *obj, void *info);

  template<typename ReturnType, typename ObjectInfo>
  static const ReturnType &asImmutable(void *obj, void *info);

  //template<typename ReturnType, typename ObjectInfo>
  //ReturnType &&asImmutable(void *obj, void *info);
};

struct BaseObjectType {
  explicit BaseObjectType(size_t size) : size_(size) {};
  const size_t size_;
};

struct DoubleType : BaseObjectType {
  DoubleType() : BaseObjectType(sizeof(double)) {};
};

struct IntType : BaseObjectType {
  IntType() : BaseObjectType(sizeof(int)) {};
};

class BaseFactoryObject {
 public:
  virtual void *createObject(BaseObjectType *) = 0;
  virtual void deleteObject(void *obj) = 0;
};

class DoubleFactoryObject : public BaseFactoryObject {
 public:
  void *createObject(BaseObjectType *) override {
    return new double;
  }
  void deleteObject(void *obj) override {
    auto tmp = static_cast<double *>(obj);
    delete tmp;
  }

};

class IntFactoryObject : public BaseFactoryObject {
  void *createObject(BaseObjectType *) override {
    return new int;
  }
  void deleteObject(void *obj) override {
    auto tmp = static_cast<int *>(obj);
    delete tmp;
  }

};

template<>
double &TraactTypeConversion::asMutable<double, DoubleType>(void *obj, void *info) {
  auto myinfo = static_cast<DoubleType *>(info);
  return *static_cast<double *>(obj);
}

template<>
const int &TraactTypeConversion::asImmutable<int, DoubleType>(void *obj, void *info) {
  auto myinfo = static_cast<DoubleType *>(info);
  int result = std::round(*static_cast<double *>(obj));
  return std::move(result);
}

template<>
int &TraactTypeConversion::asMutable<int, IntType>(void *obj, void *info) {
  auto myinfo = static_cast<IntType *>(info);
  return *static_cast<int *>(obj);
}

int main(int argc, char **argv) {
  TraactTypeConversion type_conversion;

  BaseFactoryObject *doubleFac = new DoubleFactoryObject();
  BaseObjectType *doubleInfo = new DoubleType();

  void *d1 = doubleFac->createObject(doubleInfo);

  std::cout << (typeid(DoubleType) == typeid(*doubleInfo)) << std::endl;
  std::cout << (typeid(BaseObjectType) == typeid(*doubleInfo)) << std::endl;

  double &d1r = type_conversion.asMutable<double, DoubleType>(d1, doubleInfo);
  d1r = 8;

  {
    const int &dtmp = type_conversion.asImmutable<int, DoubleType>(d1, doubleInfo);
    std::cout << dtmp << std::endl;
  }

  delete doubleInfo;
  delete doubleFac;

  return 0;
}