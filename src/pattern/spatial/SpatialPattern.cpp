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

#include "traact/pattern/spatial/SpatialPattern.h"
traact::pattern::spatial::SpatialPattern::SpatialPattern(const std::string &name, size_t concurrency)
    : Pattern(name, concurrency) {}
traact::pattern::spatial::SpatialPattern::SpatialPattern(const traact::pattern::Pattern &value) : Pattern(value) {

}
traact::pattern::spatial::SpatialPattern &traact::pattern::spatial::SpatialPattern::addCoordianteSystem(const std::string &name, bool is_multi) {
  CoordinateSystem newCoord(name, is_multi);
  coordinate_systems_.emplace(std::make_pair(name, std::move(newCoord)));
  return *this;
}
traact::pattern::spatial::SpatialPattern &traact::pattern::spatial::SpatialPattern::addEdge(const std::string &source,
                                                                                            const std::string &destination,
                                                                                            const std::string &port) {

  //TODO check for valid input
  edges_.emplace(std::make_tuple(source, destination, port));
  return *this;
}
traact::pattern::spatial::SpatialPattern::SpatialPattern() : Pattern("Invalid", -1) {

}

