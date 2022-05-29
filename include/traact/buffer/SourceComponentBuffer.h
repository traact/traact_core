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

#ifndef TRAACTMULTI_SOURCECOMPONENTBUFFER_H
#define TRAACTMULTI_SOURCECOMPONENTBUFFER_H

#include <traact/datatypes.h>
#include <vector>
#include "ComponentBuffer.h"
#include <future>
namespace traact::buffer {
    class TRAACT_CORE_EXPORT SourceComponentBuffer {
    public:
        using CommitCallback = std::function<void (SourceComponentBuffer*,bool)>;

        explicit SourceComponentBuffer(ComponentBuffer &output_buffer, const CommitCallback &callback);

        SourceComponentBuffer(SourceComponentBuffer&& rhs) = default;
        SourceComponentBuffer& operator=(SourceComponentBuffer&& rhs) = default;

        SourceComponentBuffer(const SourceComponentBuffer& rhs) = delete;
        SourceComponentBuffer& operator=(SourceComponentBuffer rhs) = delete;




        template<typename ReturnType, typename HeaderType>
        ReturnType &getOutput(size_t index) {
            return getOutput<HeaderType>(index);
        }

        template<typename HeaderType>
        typename HeaderType::NativeType &getOutput(size_t index) {
            return local_output_buffer_.template getOutput<HeaderType>(index);
        }

        template<typename Port>
        typename Port::Header::NativeType &getOutput() {
            return local_output_buffer_.template getOutput<Port>();
        }

        std::size_t GetOutputCount();

        TimestampType GetTimestamp();

        void Commit(bool valid);

        void ResetLock();
        std::future<bool> GetSourceLock();

    private:
        ComponentBuffer& local_output_buffer_;
        std::promise<bool> source_lock_;
        CommitCallback commit_callback_;
    };
}



#endif //TRAACTMULTI_SOURCECOMPONENTBUFFER_H
