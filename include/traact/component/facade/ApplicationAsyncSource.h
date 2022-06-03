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

#ifndef TRAACTMULTI_TRAACT_FACADE_INCLUDE_TRAACT_COMPONENT_FACADE_APPLICATIONASYNCSOURCE_H_
#define TRAACTMULTI_TRAACT_FACADE_INCLUDE_TRAACT_COMPONENT_FACADE_APPLICATIONASYNCSOURCE_H_

#include <traact/traact.h>
#include <traact/buffer/SourceComponentBuffer.h>
namespace traact::component::facade {

template<typename HeaderType>
class ApplicationAsyncSource : public Component {
 public:
  typedef typename std::shared_ptr<ApplicationAsyncSource<HeaderType> > Ptr;
  typedef typename HeaderType::NativeType NativeType;

  explicit ApplicationAsyncSource(const std::string &name) : Component(name, traact::component::ComponentType::AsyncSource) {

  }

  pattern::Pattern::Ptr GetPattern() const{
      std::string pattern_name = "ApplicationAsyncSource_"+std::string(HeaderType::NativeTypeName);
      pattern::Pattern::Ptr
              pattern = std::make_shared<pattern::Pattern>(pattern_name, 1);
      pattern->addProducerPort("output", HeaderType::MetaType);
      pattern->addCoordinateSystem("A").addCoordinateSystem("B").addEdge("A","B","output");
      return pattern;
  }

  bool newValue(TimestampType ts, const NativeType& value) {

      try{
          //spdlog::info("acquire buffer");
          auto buffer = request_callback_(ts);
          buffer.wait();
          auto buffer_p = buffer.get();
          if (buffer_p == nullptr)
              return false;

          //spdlog::info("get outout");
          auto &newData = buffer_p->template getOutput<NativeType, HeaderType>(0);
          //spdlog::info("write value");
          newData = value;
          //spdlog::info("commit data");
          buffer_p->Commit(true);

          return true;
      } catch(std::future_error e) {
          SPDLOG_ERROR(e.what());
          throw e;
      }

      return false;


  }

  bool start() override {
    spdlog::info("ApplicationAsyncSource got start signal");
	return true;
  }
  bool stop() override {
    spdlog::info("ApplicationAsyncSource got Stop signal");
	return true;
  }

RTTR_ENABLE( Component )

};
}



#endif //TRAACTMULTI_TRAACT_FACADE_INCLUDE_TRAACT_COMPONENT_FACADE_APPLICATIONASYNCSOURCE_H_
