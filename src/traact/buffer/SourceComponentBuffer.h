/** Copyright (C) 2022  Frieder Pankratz <frieder.pankratz@gmail.com> **/

#ifndef TRAACT_CORE_SRC_TRAACT_BUFFER_SOURCECOMPONENTBUFFER_H_
#define TRAACT_CORE_SRC_TRAACT_BUFFER_SOURCECOMPONENTBUFFER_H_

#include <traact/datatypes.h>
#include <vector>
#include "ComponentBuffer.h"
#include <future>
namespace traact::buffer {
class TRAACT_CORE_EXPORT SourceComponentBuffer {
 public:
    using CommitCallback = std::function<void(SourceComponentBuffer *, bool)>;

    explicit SourceComponentBuffer(ComponentBuffer &output_buffer);

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

    size_t getOutputCount();

    Timestamp getTimestamp();

    void commit(bool valid);
    void cancel();

    void resetLock();
    std::future<bool> getSourceLock();

 private:
    const ComponentBuffer &local_output_buffer_;
    std::promise<bool> source_lock_;
    std::atomic_bool lock_set_{false};
};
}

#endif //TRAACT_CORE_SRC_TRAACT_BUFFER_SOURCECOMPONENTBUFFER_H_
