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

#ifndef TRAACTMULTI_TRAACT_FACADE_INCLUDE_TRAACT_COMPONENT_FACADE_APPLICATIONSYNCSINK_H_
#define TRAACTMULTI_TRAACT_FACADE_INCLUDE_TRAACT_COMPONENT_FACADE_APPLICATIONSYNCSINK_H_

#include <traact/traact.h>

namespace traact::component::facade {

template<typename HeaderType>
class ApplicationSyncSink : public Component {
 public:
  typedef typename std::shared_ptr<ApplicationSyncSink<HeaderType> > Ptr;
  typedef typename HeaderType::NativeType NativeType;
  typedef typename std::function<void(TimestampType, const NativeType&)> NewValueCallback;
  typedef typename std::function<void(TimestampType, std::size_t mea_idx)> InvalidTimestampCallback;

  explicit ApplicationSyncSink(const std::string &name) : Component(name, traact::component::ComponentType::SyncSink) {

  }

    pattern::Pattern::Ptr GetPattern() const{
        std::string pattern_name = "ApplicationSyncSink_"+std::string(HeaderType::NativeTypeName);
        pattern::spatial::SpatialPattern::Ptr
                pattern = std::make_shared<pattern::spatial::SpatialPattern>(pattern_name, 1);
        pattern->addConsumerPort("input", HeaderType::MetaType);
        return pattern;
    }

  static void fillPattern(traact::pattern::Pattern::Ptr& pattern) {
    std::string pattern_name = "ApplicationSyncSink_"+std::string(HeaderType::NativeTypeName);
    pattern->name = pattern_name;
    pattern->addConsumerPort("input", HeaderType::MetaType);
  }


  bool processTimePoint(traact::DefaultComponentBuffer &data) override {

    if(callback_){
        const auto &input = data.getInput<NativeType, HeaderType>(0);
        callback_(data.getTimestamp(), input);
    } else {
        spdlog::warn("ApplicationSyncSink {0}: missing callback function", getName());
    }

	return true;
  }

    void invalidTimePoint(traact::TimestampType ts, std::size_t mea_idx) override {
        if(invalid_callback_)
            invalid_callback_(ts, mea_idx);
    }

    bool start() override {
    spdlog::info("ApplicationSyncSink got start signal");
	return true;

  }
  bool stop() override {
    spdlog::info("ApplicationSyncSink got Stop signal");
	return true;
  }

  void SetCallback(const NewValueCallback &callback) {
    callback_ = callback;
  }

    void SetInvalidCallback(const InvalidTimestampCallback &callback) {
        invalid_callback_ = callback;
    }

 protected:
  NewValueCallback callback_;
  InvalidTimestampCallback invalid_callback_;

  RTTR_ENABLE( Component )

};
}

#endif //TRAACTMULTI_TRAACT_FACADE_INCLUDE_TRAACT_COMPONENT_FACADE_APPLICATIONSYNCSINK_H_
