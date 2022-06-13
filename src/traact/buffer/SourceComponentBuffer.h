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
    ReturnType &getOutput(size_t index) const {
        return getOutput<HeaderType>(index);
    }

    template<typename HeaderType>
    typename HeaderType::NativeType &getOutput(size_t index) const {
        return local_output_buffer_.template getOutput<HeaderType>(index);
    }

    template<typename Port>
    typename Port::Header::NativeType &getOutput() const {
        return local_output_buffer_.template getOutput<Port>();
    }

    template<typename Port>
    typename Port::Header &getOutputHeader() const {
        return local_output_buffer_.template getOutputHeader<Port>();
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
};
}

#endif //TRAACT_CORE_SRC_TRAACT_BUFFER_SOURCECOMPONENTBUFFER_H_
