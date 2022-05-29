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

#ifndef TRAACT_INCLUDE_TRAACT_BUFFER_GENERIC_GENERICCOMPONENTBUFFER_H_
#define TRAACT_INCLUDE_TRAACT_BUFFER_GENERIC_GENERICCOMPONENTBUFFER_H_

#include <tuple>

#include <traact/datatypes.h>
#include <traact/traact_core_export.h>
#include <traact/util/Logging.h>
namespace traact::buffer {




class TRAACT_CORE_EXPORT ComponentBuffer {
 public:
    using LocalDataBufferType = std::vector<void*>;
    ComponentBuffer(LocalDataBufferType input_buffer, LocalDataBufferType output_buffer);
    ComponentBuffer() = default;

  template<typename ReturnType, typename HeaderType>
  const ReturnType& getInput(size_t index) const{
    return getInput<HeaderType>(index);

  }

  template<typename ReturnType, typename HeaderType>
  ReturnType &getOutput(size_t index) const{
      return getOutput<HeaderType>(index);
  }

    template<typename HeaderType>
    const typename HeaderType::NativeType& getInput(size_t index) const{
        return *static_cast<typename HeaderType::NativeType *>(local_input_buffer_[index]);
    }

    template<typename HeaderType>
    typename HeaderType::NativeType &getOutput(size_t index) const{
        return *static_cast<typename HeaderType::NativeType *>(local_output_buffer_[index]);
    }


    template<typename Port>
    const typename Port::Header::NativeType& getInput() const{
        return *static_cast<typename Port::Header::NativeType *>(local_input_buffer_[Port::PortIdx]);

    }

    template<typename Port>
    typename Port::Header::NativeType &getOutput() const{
        return *static_cast<typename Port::Header::NativeType *>(local_output_buffer_[Port::PortIdx]);
    }

    std::size_t GetInputCount();

    std::size_t GetOutputCount();

    void SetTimestamp(TimestampType ts);

    TimestampType GetTimestamp();

    private:
        TimestampType local_ts_;
        LocalDataBufferType local_input_buffer_;
        LocalDataBufferType local_output_buffer_;
    };
}

#endif //TRAACT_INCLUDE_TRAACT_BUFFER_GENERIC_GENERICCOMPONENTBUFFER_H_
