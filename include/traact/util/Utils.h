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

#ifndef TRAACT_INCLUDE_TRAACT_UTIL_UTILS_H_
#define TRAACT_INCLUDE_TRAACT_UTIL_UTILS_H_

#include <set>
#include <map>

#include <spdlog/spdlog.h>

namespace traact::util {
template<typename K, typename T>
static std::set<K> getKeys(const std::map<K, T> &values) {
  std::set<K> result;
  for (const auto &item : values) {
    result.emplace(item.first);
  }
  return std::move(result);
}
template<typename K, typename T>
static std::set<T> getValues(const std::map<K, T> &values) {
  std::set<T> result;
  for (const auto &item : values) {
    result.emplace(item.second);
  }
  return std::move(result);
}

template<typename T>
bool vectorContainsName(const std::vector<T> &vecOfElements, const std::string &name) {
  auto it = std::find_if(vecOfElements.begin(), vecOfElements.end(), [&name](const T &val) {
    if (val.getName() == name)
      return true;
    return false;
  });

  return it != vecOfElements.end();
}

template<typename T>
const T *vectorGetForName(const std::vector<T> &vecOfElements, const std::string &name) {
  auto it = std::find_if(vecOfElements.begin(), vecOfElements.end(), [name](const T &val) {
    if (val.getName() == name)
      return true;
    return false;
  });

  if (it == vecOfElements.end()) {
    spdlog::error("used vectorGetForName with unkown name");
  }

  return &(*it);
}

template<typename T>
T *vectorGetForName(std::vector<T> &vecOfElements, const std::string &name) {
  auto it = std::find_if(vecOfElements.begin(), vecOfElements.end(), [name](const T &val) {
    if (val.getName() == name)
      return true;
    return false;
  });

  if (it == vecOfElements.end()) {
    spdlog::error("used vectorGetForName with unkown name");
  }

  return &(*it);
}

}

#endif //TRAACT_INCLUDE_TRAACT_UTIL_UTILS_H_
